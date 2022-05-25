import torch
import torchvision
from PIL import Image
import numpy as np
from math import log10
import os
import random
import datetime
from config import *
from model import UNet, BackWarp

# Class containing dataset loading methods
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
                    if image[-4:] == '.jpg':
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
        self.randomCropSize = TRAIN_RANDOM_CROP_SIZE if train else VALIDATE_RANDOM_CROP_SIZE
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
            cropArea = (cropY, cropX, self.randomCropSize[1], self.randomCropSize[0])
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
            cropArea = (0, 0, self.randomCropSize[1], self.randomCropSize[0])
            IFrameIndex = ((index) % 7  + 1)
            returnIndex = IFrameIndex - 1
            frameRange = [0, IFrameIndex, 8]
            randomFrameFlip = 0
        
        for frameIndex in frameRange:
            with open(self.framesPath[index][frameIndex], 'rb') as f:
                frame = Image.open(f)
                frame = frame.crop((cropArea[1], cropArea[0], cropArea[1] + cropArea[3], cropArea[0] + cropArea[2]))
                frame = frame.transpose(Image.FLIP_LEFT_RIGHT) if randomFrameFlip else frame
                frame = FRAME_TRANSFORM(frame.convert('RGB'))
                sample.append(frame)

        # get corresponding F_0_1 and F_1_0 to sample frames
        F_0_1_path = self.flowVectorsPath[index][firstFrame]
        F_1_0_path = self.flowVectorsPath[index][firstFrame + 4]

        # load as tensors, crop, flip
        F_0_1 = self.floToTensor(F_0_1_path)
        F_1_0 = self.floToTensor(F_1_0_path)
        F_0_1 = torchvision.transforms.functional.crop(F_0_1, cropArea[0], cropArea[1], cropArea[2], cropArea[3])
        F_1_0 = torchvision.transforms.functional.crop(F_1_0, cropArea[0], cropArea[1], cropArea[2], cropArea[3])
        F_0_1 = torch.flip(F_0_1, [3]) if randomFrameFlip else F_0_1
        F_1_0 = torch.flip(F_1_0, [3]) if randomFrameFlip else F_1_0

        # append flow vectors to the end of sample
        sample.append(F_0_1)
        sample.append(F_1_0)
            
        return sample, returnIndex

    def __len__(self):
        return len(self.framesPath)

    def floToTensor(floFile):
        dummy = np.fromfile(floFile, dtype=np.dtype(np.float32), count = 1, offset = 0)[0]
        width = np.fromfile(floFile, dtype=np.dtype(np.int32), count = 1, offset = 4)[0]
        height = np.fromfile(floFile, dtype=np.dtype(np.int32), count = 1, offset = 8)[0]
        if (dummy != 202021.25):
            print("Something went wrong with the .flo extraction, first 4 bytes are not equal to 202021.25")
            return

        floValues = np.fromfile(floFile, dtype=np.dtype(np.float32), count = -1, offset = 12)

        xFlo = floValues[0::2]
        yFlo = floValues[1::2]

        xTensor = torch.from_numpy(xFlo)
        xTensor = torch.reshape(xTensor, (1,height,width))

        yTensor = torch.from_numpy(yFlo)
        yTensor = torch.reshape(yTensor, (1,height,width))

        floTensor = torch.cat((xTensor,yTensor),0)
        return floTensor

    def yuvToRGBTensor(yuvFile):
    # helper function to map U&V Values to the same size as the Y Tensor
        def mapColor(colorValues, width):
            colorTensor = torch.empty(0, dtype=torch.uint8)
            rowTensor = torch.empty(0, dtype=torch.uint8)

            for i in range(colorValues.size):
                valueTensor = torch.full((2,2), colorValues[i], dtype=torch.uint8)
                rowTensor = torch.cat((rowTensor,valueTensor),1)

                if rowTensor.shape[1] == width:
                    colorTensor = torch.cat((colorTensor, rowTensor),0)
                    rowTensor = torch.empty(0, dtype=torch.uint8)
        
            return colorTensor

        width = IMAGE_DIM[0]
        height = IMAGE_DIM[1]
        imgSize = width * height

        yValues = np.fromfile(yuvFile, dtype=np.dtype('b'), count = imgSize, offset = 0)
        uValues = np.fromfile(yuvFile, dtype=np.dtype('b'), count = imgSize//4, offset = imgSize)
        vValues = np.fromfile(yuvFile, dtype=np.dtype('b'), count = imgSize//4, offset = imgSize + (imgSize//4))
    
        yValues = yValues.astype("uint8")
        uValues = uValues.astype("uint8")
        vValues = vValues.astype("uint8")

        yTensor = torch.from_numpy(yValues)
        yTensor = torch.reshape(yTensor, (height,width))
        yTensor = yTensor.type(torch.float64)

        uTensor = mapColor(uValues,width)
        uTensor = uTensor.type(torch.float64)

        vTensor = mapColor(vValues,width)
        vTensor = vTensor.type(torch.float64)

    # conversions for jpeg from: https://www.w3.org/Graphics/JPEG/jfif3.pdf
        R = yTensor + 1.40200 * (vTensor - 128.0)
        G = yTensor - .34414 * (uTensor - 128.0) - .71414 * (vTensor - 128.0)
        B = yTensor + 1.77200 * (uTensor - 128.0)

        R = torch.clamp(R, min=0, max =255)
        G = torch.clamp(G, min=0, max =255)
        B = torch.clamp(B, min=0, max =255)
        R = R.type(torch.uint8).unsqueeze_(0)
        G = G.type(torch.uint8).unsqueeze_(0)
        B = B.type(torch.uint8).unsqueeze_(0)

        rgbTensor = torch.cat((R,G,B),0)

        rgbTensor.unsqueeze_(0)

        return rgbTensor

# Class containing all training methods
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
        return torch.Tensor(C00)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C01)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C10)[None, None, None, :].permute(3, 0, 1, 2).to(self.device), torch.Tensor(C11)[None, None, None, :].permute(3, 0, 1, 2).to(self.device)

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
                loss = 204 * recnLoss + 102 * warpLoss + 0.005 * prcpLoss
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

def main():
    mainTrainRoutine = TrainRoutine()
    print ('Success with Initiation')
    mainTrainRoutine.train()

if __name__ == "__main__":
    main()
