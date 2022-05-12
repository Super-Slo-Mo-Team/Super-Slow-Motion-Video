import torch
import torchvision
from PIL import Image
from math import log10
import os
import random
import datetime
from config import *
from model import UNet, BackWarp

class DataSet(torch.utils.data.Dataset):
    def __init__(self, root, train):
        framesPath = []
        flowVectorsPath = []

        for i, folder in enumerate(os.listdir(root)):
            clipsFolderPath = os.path.join(root, folder)

            if os.path.isdir(clipsFolderPath):
                framesPath.append([])
                flowVectorsPath.append([])
                for image in sorted(os.listdir(clipsFolderPath)):
                    if image[-4:] == '.yuv':
                        framesPath[i].append(os.path.join(clipsFolderPath, image))
                    elif image[-4:] == '.flo':
                        flowVectorsPath[i].append(os.path.join(clipsFolderPath, image))
                    else:
                        raise(RuntimeError('Extra files in: ' + clipsFolderPath + '\n'))
                if len(flowVectorsPath[i]) == 0:
                    raise(RuntimeError('Flow vectors not generated for a folder in: ' + clipsFolderPath + '\n'))
        
        if len(framesPath) == 0:
            raise(RuntimeError('Found 0 files in clip folders of: ' + root + '\n'))
                
        self.root = root
        self.train = train
        if train:
            self.randomCropSize = TRAIN_RANDOM_CROP_SIZE
        else:
            self.randomCropSize = VALIDATE_RANDOM_CROP_SIZE
        self.cropX0 = IMAGE_DIM[0] - self.randomCropSize[0]
        self.cropY0 = IMAGE_DIM[1] - self.randomCropSize[1]
        self.framesPath = framesPath
        self.flowVectorsPath = flowVectorsPath

    def __getitem__(self, index):
        sample = []
        
        if (self.train):
            firstFrame = random.randint(0, 3)
            cropX = random.randint(0, self.cropX0)
            cropY = random.randint(0, self.cropY0)
            cropArea = (cropX, cropY, cropX + self.randomCropSize[0], cropY + self.randomCropSize[1])
            IFrameIndex = random.randint(firstFrame + 1, firstFrame + 7)
            if (random.randint(0, 1)):
                frameRange = [firstFrame, IFrameIndex, firstFrame + 8]
                returnIndex = IFrameIndex - firstFrame - 1
            else:
                frameRange = [firstFrame + 8, IFrameIndex, firstFrame]
                returnIndex = firstFrame - IFrameIndex + 7
            randomFrameFlip = random.randint(0, 1)
        else:
            firstFrame = 0
            cropArea = (0, 0, self.randomCropSize[0], self.randomCropSize[1])
            IFrameIndex = ((index) % 7  + 1)
            returnIndex = IFrameIndex - 1
            frameRange = [0, IFrameIndex, 8]
            randomFrameFlip = 0
        
        # TODO: .yuv

        for frameIndex in frameRange:
            image = None
            with open(self.framesPath[index][frameIndex], 'rb') as f:
                img = Image.open(f)
                cropped_img = img.crop(cropArea) if (cropArea != None) else img
                flipped_img = cropped_img.transpose(Image.FLIP_LEFT_RIGHT) if randomFrameFlip else cropped_img
                image = flipped_img.convert('YCbCr')

            sample.append(image)

        # get corresponding F_0_1 and F_1_0 to sample frames
        F_0_1_path = self.flowVectorsPath[index][2 * firstFrame]
        F_1_0_path = self.flowVectorsPath[index][2 * firstFrame + 1]

        # TODO: load as tensors, crop, maybe flip
        # https://stackoverflow.com/questions/28013200/reading-middlebury-flow-files-with-python-bytes-array-numpy
        F_0_1 = None
        F_1_0 = None

        # append flow vectors to the end of sample
        sample.append(F_0_1)
        sample.append(F_1_0)
            
        return sample, returnIndex

    def __len__(self):
        return len(self.framesPath)

class TrainRoutine():
    def __init__(self):
        # initialize device
        self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')

        # initialize interpolationModel CNN
        self.interpolationModel = UNet()
        self.interpolationModel.to(self.device)

        # initialize backwarp function
        self.trainBackWarp = BackWarp(TRAIN_RANDOM_CROP_SIZE, self.device)
        self.trainBackWarp = self.trainBackWarp.to(self.device)
        self.validationBackWarp = BackWarp(VALIDATE_RANDOM_CROP_SIZE, self.device)
        self.validationBackWarp = self.validationBackWarp.to(self.device)

        # load train and validation sets
        trainSet = DataSet(root = TRAINING_TRAIN_PATH, train = True)
        self.trainLoader = torch.utils.data.DataLoader(trainSet, batch_size = TRAIN_BATCH_SIZE, shuffle = True)
        validationSet = DataSet(root = TRAINING_VALIDATE_PATH, train = False)
        self.validationLoader = torch.utils.data.DataLoader(validationSet, batch_size = VALIDATION_BATCH_SIZE, shuffle = False)

        # loss and optimizer
        self.L1_lossFn = torch.nn.L1Loss()
        self.MSE_lossFn = torch.nn.MSELoss()
        self.optimizer = torch.optim.Adam(list(self.interpolationModel.parameters()), lr = LEARNING_RATE)
        self.scheduler = torch.optim.lr_scheduler.MultiStepLR(self.optimizer, milestones = MILESTONES, gamma = 0.1)

        # initialize VGG16 model for perceptual loss
        vgg16 = torchvision.models.vgg16(pretrained = True)
        self.vgg16_conv_4_3 = torch.nn.Sequential(*list(vgg16.children())[0][:22])
        self.vgg16_conv_4_3.to(self.device)
        for param in self.vgg16_conv_4_3.parameters():
            param.requires_grad = False

        # initialize model dict
        self.cLoss = []
        self.valLoss = []
        self.valPSNR = []
        self.checkpoint_counter = 0

    # helper function to calculate coefficients for F_t_0 and F_t_1
    def getFlowCoeff(self, indices):
        ind = indices.detach().numpy()
        C11 = C00 = - (1 - (TSTEPS[ind])) * (TSTEPS[ind])
        C01 = (TSTEPS[ind]) * (TSTEPS[ind])
        C10 = (1 - (TSTEPS[ind])) * (1 - (TSTEPS[ind]))
        return torch.Tensor(C00)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C01)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C10)[None, None, None, :].permute(3, 0, 1, 2).to(device), torch.Tensor(C11)[None, None, None, :].permute(3, 0, 1, 2).to(device)

    # helper function to get coefficients to calculate final intermediate frame
    def getWarpCoeff(self, indices):
        ind = indices.detach().numpy()
        C0 = 1 - TSTEPS[ind]
        C1 = TSTEPS[ind]
        return torch.Tensor(C0)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C1)[None, None, None, :].permute(3, 0, 1, 2).to(self.device)

    # helper function to get learning rate
    def get_lr(self, optimizer):
        for param_group in optimizer.param_groups:
            return param_group['lr']

    #
    # Validation routine
    #
    def validate(self):
        psnr = 0
        tloss = 0
        
        with torch.no_grad():
            for _, (validationData, validationFrameIndex) in enumerate(self.validationLoader, 0):
                frame0, frameT, frame1, F_0_1, F_1_0 = validationData

                I0 = frame0.to(self.device)
                I1 = frame1.to(self.device)
                IFrame = frameT.to(self.device)
                F_0_1 = F_0_1.to(self.device)
                F_1_0 = F_1_0.to(self.device)

                fCoeff = self.getFlowCoeff(validationFrameIndex)

                F_t_0 = fCoeff[0] * F_0_1 + fCoeff[1] * F_1_0
                F_t_1 = fCoeff[2] * F_0_1 + fCoeff[3] * F_1_0

                g_I0_F_t_0 = self.validationBackWarp(I0, F_t_0)
                g_I1_F_t_1 = self.validationBackWarp(I1, F_t_1)

                interpolationRes = self.interpolationModel(torch.cat((I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0), dim = 1))

                F_t_0_f = interpolationRes[:, :2, :, :] + F_t_0
                F_t_1_f = interpolationRes[:, 2:4, :, :] + F_t_1
                V_t_0 = interpolationRes[:, 4, :, :]
                V_t_1 = 1 - V_t_0
                    
                g_I0_F_t_0_f = self.validationBackWarp(I0, F_t_0_f)
                g_I1_F_t_1_f = self.validationBackWarp(I1, F_t_1_f)
                
                wCoeff = self.getWarpCoeff(validationFrameIndex)
                
                Ft_p = (wCoeff[0] * V_t_0 * g_I0_F_t_0_f + wCoeff[1] * V_t_1 * g_I1_F_t_1_f) / (wCoeff[0] * V_t_0 + wCoeff[1] * V_t_1)
                
                # loss
                recnLoss = self.L1_lossFn(Ft_p, IFrame)
                prcpLoss = self.MSE_lossFn(self.vgg16_conv_4_3(Ft_p), self.vgg16_conv_4_3(IFrame))
                warpLoss = self.L1_lossFn(g_I0_F_t_0, IFrame) + self.L1_lossFn(g_I1_F_t_1, IFrame) + self.L1_lossFn(self.validationBackWarp(I0, F_1_0), I1) + self.L1_lossFn(self.validationBackWarp(I1, F_0_1), I0)
                loss_smooth_1_0 = torch.mean(torch.abs(F_1_0[:, :, :, :-1] - F_1_0[:, :, :, 1:])) + torch.mean(torch.abs(F_1_0[:, :, :-1, :] - F_1_0[:, :, 1:, :]))
                loss_smooth_0_1 = torch.mean(torch.abs(F_0_1[:, :, :, :-1] - F_0_1[:, :, :, 1:])) + torch.mean(torch.abs(F_0_1[:, :, :-1, :] - F_0_1[:, :, 1:, :]))
                loss_smooth = loss_smooth_1_0 + loss_smooth_0_1
                loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss + loss_smooth
                tloss += loss.item()

                # psnr
                MSE_val = self.MSE_lossFn(Ft_p, IFrame)
                psnr += (10 * log10(1 / MSE_val.item()))

        return (psnr / len(self.validationLoader)), (tloss / len(self.validationLoader))

    #
    # Training routine
    #
    def train(self):
        for epoch in range(NUM_EPOCHS):
            print ('Epoch: ', epoch)
                
            # append and reset
            self.cLoss.append([])
            self.valLoss.append([])
            self.valPSNR.append([])
            iLoss = 0
            
            # increment scheduler count    
            self.scheduler.step()
            
            for trainIndex, (trainData, trainFrameIndex) in enumerate(self.trainLoader, 0):
                frame0, frameT, frame1, F_0_1, F_1_0 = trainData
                
                I0 = frame0.to(self.device)
                I1 = frame1.to(self.device)
                IFrame = frameT.to(self.device)
                F_0_1 = F_0_1.to(self.device)
                F_1_0 = F_1_0.to(self.device)
                
                self.optimizer.zero_grad()
                
                fCoeff = self.getFlowCoeff(trainFrameIndex)
                
                # calculate intermediate flows
                F_t_0 = fCoeff[0] * F_0_1 + fCoeff[1] * F_1_0
                F_t_1 = fCoeff[2] * F_0_1 + fCoeff[3] * F_1_0
                
                # get intermediate frames from the intermediate flows
                g_I0_F_t_0 = self.trainBackWarp(I0, F_t_0)
                g_I1_F_t_1 = self.trainBackWarp(I1, F_t_1)
                
                # calculate optical flow residuals and visibility maps
                interpolationRes = self.interpolationModel(torch.cat((I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0), dim = 1))
                
                # extract optical flow residuals and visibility maps
                F_t_0_f = interpolationRes[:, :2, :, :] + F_t_0
                F_t_1_f = interpolationRes[:, 2:4, :, :] + F_t_1
                V_t_0 = interpolationRes[:, 4, :, :]
                V_t_1 = 1 - V_t_0
                
                # get intermediate frames from the intermediate flows
                g_I0_F_t_0_f = self.trainBackWarp(I0, F_t_0_f)
                g_I1_F_t_1_f = self.trainBackWarp(I1, F_t_1_f)
                
                wCoeff = self.getWarpCoeff(trainFrameIndex)
                
                # calculate final intermediate frame 
                Ft_p = (wCoeff[0] * V_t_0 * g_I0_F_t_0_f + wCoeff[1] * V_t_1 * g_I1_F_t_1_f) / (wCoeff[0] * V_t_0 + wCoeff[1] * V_t_1)
                
                # loss
                recnLoss = self.L1_lossFn(Ft_p, IFrame)
                prcpLoss = self.MSE_lossFn(self.vgg16_conv_4_3(Ft_p), self.vgg16_conv_4_3(IFrame))
                warpLoss = self.L1_lossFn(g_I0_F_t_0, IFrame) + self.L1_lossFn(g_I1_F_t_1, IFrame) + self.L1_lossFn(self.trainBackWarp(I0, F_1_0), I1) + self.L1_lossFn(self.trainBackWarp(I1, F_0_1), I0)
                
                # TODO: is smoothness loss necessary?
                
                # loss_smooth_1_0 = torch.mean(torch.abs(F_1_0[:, :, :, :-1] - F_1_0[:, :, :, 1:])) + torch.mean(torch.abs(F_1_0[:, :, :-1, :] - F_1_0[:, :, 1:, :]))
                # loss_smooth_0_1 = torch.mean(torch.abs(F_0_1[:, :, :, :-1] - F_0_1[:, :, :, 1:])) + torch.mean(torch.abs(F_0_1[:, :, :-1, :] - F_0_1[:, :, 1:, :]))
                # loss_smooth = loss_smooth_1_0 + loss_smooth_0_1
                
                # TODO: change coefficients because yuv not rgb

                # loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss + loss_smooth
                loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss

                # backpropagate
                loss.backward()
                self.optimizer.step()
                iLoss += loss.item()

                # validation and progress every 100 iterations
                if ((trainIndex % NUM_ITERATIONS) == NUM_ITERATIONS - 1):
                    psnr, vLoss = self.validate()

                    self.valPSNR[epoch].append(psnr)
                    self.valLoss[epoch].append(vLoss)

                    print('Loss: %0.6f  Iterations: %4d/%4d  ValLoss:%0.6f  ValPSNR: %0.4f  LearningRate: %f' % (iLoss / NUM_ITERATIONS, trainIndex, len(self.trainLoader), vLoss, psnr, self.get_lr(self.optimizer)))

                    self.cLoss[epoch].append(iLoss / NUM_ITERATIONS)
                    iLoss = 0

            # Create checkpoint every CHECKPOINT_EPOCH
            if ((epoch % CHECKPOINT_EPOCH) == CHECKPOINT_EPOCH - 1):
                interpolationDict = {
                    'Detail' : '',
                    'epoch' : epoch,
                    'timestamp' : datetime.datetime.now(),
                    'trainBatchSz' : TRAIN_BATCH_SIZE,
                    'validationBatchSz' : VALIDATION_BATCH_SIZE,
                    'learningRate' : self.get_lr(self.optimizer),
                    'loss' : self.cLoss,
                    'valLoss' : self.valLoss,
                    'valPSNR' : self.valPSNR,
                    'state_dict' : self.interpolationModel.state_dict()
                }

                torch.save(interpolationDict, TRAINING_CHECKPOINT_PATH + '/epoch_' + str(checkpoint_counter) + '.ckpt')
                checkpoint_counter += 1
