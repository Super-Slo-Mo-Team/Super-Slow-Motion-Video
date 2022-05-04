import torch
import torchvision
import numpy as np
from PIL import Image
from math import log10
import os
import random
import datetime
from config import *
from model import UNet, BackWarp

class DataSet(torch.utils.data.Dataset):
    def __init__(self, root, transform = None, dim = (640, 360), randomCropSize = (352, 352), train = True):
        framesPath = []

        for i, folder in enumerate(os.listdir(root)):
            clipsFolderPath = os.path.join(root, folder)

            if os.path.isdir(clipsFolderPath):
                framesPath.append([])
                for image in sorted(os.listdir(clipsFolderPath)):
                    framesPath[i].append(os.path.join(clipsFolderPath, image))
        
        if len(framesPath) == 0:
            raise(RuntimeError("Found 0 files in clip folders of: " + root + "\n"))
                
        self.root = root
        self.transform = transform
        self.randomCropSize = randomCropSize
        self.cropX0 = dim[0] - randomCropSize[0]
        self.cropY0 = dim[1] - randomCropSize[1]
        self.train = train
        self.framesPath = framesPath

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
        
        for frameIndex in frameRange:
            image = None
            with open(self.framesPath[index][frameIndex], 'rb') as f:
                img = Image.open(f)
                cropped_img = img.crop(cropArea) if (cropArea != None) else img
                flipped_img = cropped_img.transpose(Image.FLIP_LEFT_RIGHT) if randomFrameFlip else cropped_img
                image = flipped_img.convert('YCbCr')

            if self.transform is not None:
                image = self.transform(image)

            sample.append(image)

        # TODO: generate F_0_1 and F_1_0
        # F_0_1 = ...(sample[0], sample[-1])
        # F_1_0 = ...(sample[0], sample[-1])
        # TODO: crop, flip, transform
        # sample.append(F_0_1)
        # sample.append(F_1_0)
            
        return sample, returnIndex

    def __len__(self):
        return len(self.framesPath)

#
# Main training routine
#
def train():
    # helper function to calculate coefficients for F_t_0 and F_t_1
    def getFlowCoeff(indices, device):
        ind = indices.detach().numpy()
        C11 = C00 = - (1 - (t[ind])) * (t[ind])
        C01 = (t[ind]) * (t[ind])
        C10 = (1 - (t[ind])) * (1 - (t[ind]))
        return torch.Tensor(C00)[None, None, None, :].permute(3, 0, 1, 2).to(device), torch.Tensor(C01)[None, None, None, :].permute(3, 0, 1, 2).to(device), torch.Tensor(C10)[None, None, None, :].permute(3, 0, 1, 2).to(device), torch.Tensor(C11)[None, None, None, :].permute(3, 0, 1, 2).to(device)

    # helper function to get coefficients to calculate final intermediate frame
    def getWarpCoeff(indices, device):
        ind = indices.detach().numpy()
        C0 = 1 - t[ind]
        C1 = t[ind]
        return torch.Tensor(C0)[None, None, None, :].permute(3, 0, 1, 2).to(device), torch.Tensor(C1)[None, None, None, :].permute(3, 0, 1, 2).to(device)

    # helper function to get learning rate
    def get_lr(optimizer):
        for param_group in optimizer.param_groups:
            return param_group['lr']

    # validation function
    def validate():
        psnr = 0
        tloss = 0
        
        with torch.no_grad():
            for _, (validationData, validationFrameIndex) in enumerate(validationLoader, 0):
                frame0, frameT, frame1, F_0_1, F_1_0 = validationData

                I0 = frame0.to(device)
                I1 = frame1.to(device)
                IFrame = frameT.to(device)
                F_0_1 = F_0_1.to(device)
                F_1_0 = F_1_0.to(device)

                fCoeff = getFlowCoeff(validationFrameIndex, device)

                F_t_0 = fCoeff[0] * F_0_1 + fCoeff[1] * F_1_0
                F_t_1 = fCoeff[2] * F_0_1 + fCoeff[3] * F_1_0

                g_I0_F_t_0 = validationBackWarp(I0, F_t_0)
                g_I1_F_t_1 = validationBackWarp(I1, F_t_1)

                interpolationRes = interpolationModel(torch.cat((I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0), dim = 1))

                F_t_0_f = interpolationRes[:, :2, :, :] + F_t_0
                F_t_1_f = interpolationRes[:, 2:4, :, :] + F_t_1
                V_t_0 = interpolationRes[:, 4, :, :]
                V_t_1 = 1 - V_t_0
                    
                g_I0_F_t_0_f = validationBackWarp(I0, F_t_0_f)
                g_I1_F_t_1_f = validationBackWarp(I1, F_t_1_f)
                
                wCoeff = getWarpCoeff(validationFrameIndex, device)
                
                Ft_p = (wCoeff[0] * V_t_0 * g_I0_F_t_0_f + wCoeff[1] * V_t_1 * g_I1_F_t_1_f) / (wCoeff[0] * V_t_0 + wCoeff[1] * V_t_1)
                
                # loss
                recnLoss = L1_lossFn(Ft_p, IFrame)
                prcpLoss = MSE_LossFn(vgg16_conv_4_3(Ft_p), vgg16_conv_4_3(IFrame))
                warpLoss = L1_lossFn(g_I0_F_t_0, IFrame) + L1_lossFn(g_I1_F_t_1, IFrame) + L1_lossFn(validationBackWarp(I0, F_1_0), I1) + L1_lossFn(validationBackWarp(I1, F_0_1), I0)
                loss_smooth_1_0 = torch.mean(torch.abs(F_1_0[:, :, :, :-1] - F_1_0[:, :, :, 1:])) + torch.mean(torch.abs(F_1_0[:, :, :-1, :] - F_1_0[:, :, 1:, :]))
                loss_smooth_0_1 = torch.mean(torch.abs(F_0_1[:, :, :, :-1] - F_0_1[:, :, :, 1:])) + torch.mean(torch.abs(F_0_1[:, :, :-1, :] - F_0_1[:, :, 1:, :]))
                loss_smooth = loss_smooth_1_0 + loss_smooth_0_1
                loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss + loss_smooth
                tloss += loss.item()

                # psnr
                MSE_val = MSE_LossFn(Ft_p, IFrame)
                psnr += (10 * log10(1 / MSE_val.item()))
                
        return (psnr / len(validationLoader)), (tloss / len(validationLoader))

    # initialize device
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    # initialize interpolationModel CNN
    interpolationModel = UNet()
    interpolationModel.to(device)

    # initialize backwarp function
    trainBackWarp = BackWarp(352, 352, device)
    validationBackWarp = BackWarp(640, 352, device)
    trainBackWarp = trainBackWarp.to(device)
    validationBackWarp = validationBackWarp.to(device)

    # array of timesteps for intermediate frame calculations
    t = np.linspace(0.125, 0.875, 7)

    # channel wise mean calculated on adobe240-fps training dataset
    mean = [0.429, 0.431, 0.397]
    std = [1, 1, 1]
    normalize = torchvision.transforms.Normalize(mean = mean, std = std)
    transform = torchvision.transforms.Compose([torchvision.transforms.ToTensor(), normalize])

    # load train and validation sets
    trainSet = DataSet(root = TRAINING_TRAIN_PATH, transform = transform, train = True)
    trainLoader = torch.utils.data.DataLoader(trainSet, batch_size = TRAIN_BATCH_SIZE, shuffle = True)
    validationSet = DataSet(root = TRAINING_VALIDATE_PATH, transform = transform, randomCropSize = (640, 352), train = False)
    validationLoader = torch.utils.data.DataLoader(validationSet, batch_size = VALIDATION_BATCH_SIZE, shuffle = False)

    # loss and Optimizer
    L1_lossFn = torch.nn.L1Loss()
    MSE_LossFn = torch.nn.MSELoss()
    optimizer = torch.optim.Adam(list(interpolationModel.parameters()), lr = LEARNING_RATE)
    scheduler = torch.optim.lr_scheduler.MultiStepLR(optimizer, milestones = MILESTONES, gamma = 0.1)

    # initialize VGG16 model for perceptual loss
    vgg16 = torchvision.models.vgg16(pretrained = True)
    vgg16_conv_4_3 = torch.nn.Sequential(*list(vgg16.children())[0][:22])
    vgg16_conv_4_3.to(device)
    for param in vgg16_conv_4_3.parameters():
        param.requires_grad = False

    # initialize model dict
    cLoss = []
    valLoss = []
    valPSNR = []
    checkpoint_counter = 0

    # main training loop
    for epoch in range(NUM_EPOCHS):
        print ("Epoch: ", epoch)
            
        # append and reset
        cLoss.append([])
        valLoss.append([])
        valPSNR.append([])
        iLoss = 0
        
        # Increment scheduler count    
        scheduler.step()
        
        for trainIndex, (trainData, trainFrameIndex) in enumerate(trainLoader, 0):
            # get the input and the target from the training set
            frame0, frameT, frame1, F_0_1, F_1_0 = trainData
            
            I0 = frame0.to(device)
            I1 = frame1.to(device)
            IFrame = frameT.to(device)
            F_0_1 = F_0_1.to(device)
            F_1_0 = F_1_0.to(device)
            
            optimizer.zero_grad()
            
            fCoeff = getFlowCoeff(trainFrameIndex, device)
            
            # calculate intermediate flows
            F_t_0 = fCoeff[0] * F_0_1 + fCoeff[1] * F_1_0
            F_t_1 = fCoeff[2] * F_0_1 + fCoeff[3] * F_1_0
            
            # get intermediate frames from the intermediate flows
            g_I0_F_t_0 = trainBackWarp(I0, F_t_0)
            g_I1_F_t_1 = trainBackWarp(I1, F_t_1)
            
            # calculate optical flow residuals and visibility maps
            interpolationRes = interpolationModel(torch.cat((I0, I1, F_0_1, F_1_0, F_t_1, F_t_0, g_I1_F_t_1, g_I0_F_t_0), dim = 1))
            
            # extract optical flow residuals and visibility maps
            F_t_0_f = interpolationRes[:, :2, :, :] + F_t_0
            F_t_1_f = interpolationRes[:, 2:4, :, :] + F_t_1
            V_t_0 = interpolationRes[:, 4, :, :]
            V_t_1 = 1 - V_t_0
            
            # get intermediate frames from the intermediate flows
            g_I0_F_t_0_f = trainBackWarp(I0, F_t_0_f)
            g_I1_F_t_1_f = trainBackWarp(I1, F_t_1_f)
            
            wCoeff = getWarpCoeff(trainFrameIndex, device)
            
            # calculate final intermediate frame 
            Ft_p = (wCoeff[0] * V_t_0 * g_I0_F_t_0_f + wCoeff[1] * V_t_1 * g_I1_F_t_1_f) / (wCoeff[0] * V_t_0 + wCoeff[1] * V_t_1)
            
            # loss
            recnLoss = L1_lossFn(Ft_p, IFrame)
            prcpLoss = MSE_LossFn(vgg16_conv_4_3(Ft_p), vgg16_conv_4_3(IFrame))
            warpLoss = L1_lossFn(g_I0_F_t_0, IFrame) + L1_lossFn(g_I1_F_t_1, IFrame) + L1_lossFn(trainBackWarp(I0, F_1_0), I1) + L1_lossFn(trainBackWarp(I1, F_0_1), I0)
            loss_smooth_1_0 = torch.mean(torch.abs(F_1_0[:, :, :, :-1] - F_1_0[:, :, :, 1:])) + torch.mean(torch.abs(F_1_0[:, :, :-1, :] - F_1_0[:, :, 1:, :]))
            loss_smooth_0_1 = torch.mean(torch.abs(F_0_1[:, :, :, :-1] - F_0_1[:, :, :, 1:])) + torch.mean(torch.abs(F_0_1[:, :, :-1, :] - F_0_1[:, :, 1:, :]))
            loss_smooth = loss_smooth_1_0 + loss_smooth_0_1
            loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss + loss_smooth

            # backpropagate
            loss.backward()
            optimizer.step()
            iLoss += loss.item()

            # validation and progress every 100 iterations
            if ((trainIndex % NUM_ITERATIONS) == NUM_ITERATIONS - 1):
                psnr, vLoss = validate()

                valPSNR[epoch].append(psnr)
                valLoss[epoch].append(vLoss)

                print("Loss: %0.6f  Iterations: %4d/%4d  ValLoss:%0.6f  ValPSNR: %0.4f  LearningRate: %f" % (iLoss / NUM_ITERATIONS, trainIndex, len(trainLoader), vLoss, psnr, get_lr(optimizer)))

                cLoss[epoch].append(iLoss / NUM_ITERATIONS)
                iLoss = 0

        # Create checkpoint every CHECKPOINT_EPOCH
        if ((epoch % CHECKPOINT_EPOCH) == CHECKPOINT_EPOCH - 1):
            interpolationDict = {
                'Detail' : '',
                'epoch' : epoch,
                'timestamp' : datetime.datetime.now(),
                'trainBatchSz' : TRAIN_BATCH_SIZE,
                'validationBatchSz' : VALIDATION_BATCH_SIZE,
                'learningRate' : get_lr(optimizer),
                'loss' : cLoss,
                'valLoss' : valLoss,
                'valPSNR' : valPSNR,
                'state_dict' : interpolationModel.state_dict()
            }

            torch.save(interpolationDict, TRAINING_CHECKPOINT_PATH + '/epoch_' + str(checkpoint_counter) + '.ckpt')
            checkpoint_counter += 1

train()
