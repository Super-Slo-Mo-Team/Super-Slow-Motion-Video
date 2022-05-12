import torch
import numpy as np
from config import *

class UNet(torch.nn.Module):
    def __init__(self):
        super(UNet, self).__init__()
        
        ### Functions ###
        self.leaky_relu = torch.nn.LeakyReLU(0.1, inplace = False)
        self.avgpool1 = torch.nn.AvgPool2d(kernel_size = 7, stride = 2, padding = 3)
        self.avgpool2 = torch.nn.AvgPool2d(kernel_size = 5, stride = 2, padding = 2)
        self.avgpool3 = torch.nn.AvgPool2d(kernel_size = 3, stride = 2, padding = 1)
        self.upsample = torch.nn.Upsample(scale_factor = 2, mode = 'bilinear')
        self.sigmoid = torch.nn.Sigmoid()
        
        ### Encoder Layers ###
        self.c1 = torch.nn.Conv2d(20, 32, kernel_size = 7, stride = 1, padding = 3, bias = False)
        self.c2 = torch.nn.Conv2d(32, 32, kernel_size = 7, stride = 1, padding = 3, bias = False)
        self.c3 = torch.nn.Conv2d(32, 64, kernel_size = 5, stride = 1, padding = 2, bias = False)
        self.c4 = torch.nn.Conv2d(64, 64, kernel_size = 5, stride = 1, padding = 2, bias = False)
        self.c5 = torch.nn.Conv2d(64, 128, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c6 = torch.nn.Conv2d(128, 128, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c7 = torch.nn.Conv2d(128, 256, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c8 = torch.nn.Conv2d(256, 256, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c9 = torch.nn.Conv2d(256, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c10 = torch.nn.Conv2d(512, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c11 = torch.nn.Conv2d(512, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c12 = torch.nn.Conv2d(512, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)

        ### Decoder Layers ###
        self.c13 = torch.nn.Conv2d(512, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c14 = torch.nn.Conv2d(512, 512, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c15 = torch.nn.Conv2d(512, 256, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c16 = torch.nn.Conv2d(256, 256, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c17 = torch.nn.Conv2d(256, 128, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c18 = torch.nn.Conv2d(128, 128, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c19 = torch.nn.Conv2d(128, 64, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c20 = torch.nn.Conv2d(64, 64, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c21 = torch.nn.Conv2d(64, 32, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c22 = torch.nn.Conv2d(32, 32, kernel_size = 3, stride = 1, padding = 1, bias = False)
        self.c23 = torch.nn.Conv2d(32, 5, kernel_size = 3, stride = 1, padding = 1, bias = False)

    def forward(self, x):
        
        ### Encoder ###
        x = self.c1(x)
        x = self.leaky_relu(x)
        x = self.c2(x)
        x = self.leaky_relu(x)
        skp1 = x

        x = self.avgpool1(x)
        x = self.c3(x)
        x = self.leaky_relu(x)
        x = self.c4(x)
        x = self.leaky_relu(x)
        skp2 = x

        x = self.avgpool2(x)
        x = self.c5(x)
        x = self.leaky_relu(x)
        x = self.c6(x)
        x = self.leaky_relu(x)
        skp3 = x

        x = self.avgpool3(x)
        x = self.c7(x)
        x = self.leaky_relu(x)
        x = self.c8(x)
        x = self.leaky_relu(x)
        skp4 = x

        x = self.avgpool3(x)
        x = self.c9(x)
        x = self.leaky_relu(x)
        x = self.c10(x)
        x = self.leaky_relu(x)
        skp5 = x

        x = self.avgpool3(x)
        x = self.c11(x)
        x = self.leaky_relu(x)
        x = self.c12(x)
        x = self.leaky_relu(x)

		### Decoder ###
        x = self.upsample(x)
        x = self.c13(x)
        x = self.leaky_relu(x)
        x = torch.cat((x, skp5), 1)
        x = self.c14(x)
        x = self.leaky_relu(x)

        x = self.upsample(x)
        x = self.c15(x)
        x = self.leaky_relu(x)
        x = torch.cat((x, skp4), 1)
        x = self.c16(x)
        x = self.leaky_relu(x)

        x = self.upsample(x)
        x = self.c17(x)
        x = self.leaky_relu(x)
        x = torch.cat((x, skp3), 1)
        x = self.c18(x)
        x = self.leaky_relu(x)

        x = self.upsample(x)
        x = self.c19(x)
        x = self.leaky_relu(x)
        x = torch.cat((x, skp2), 1)
        x = self.c20(x)
        x = self.leaky_relu(x)

        x = self.upsample(x)
        x = self.c21(x)
        x = self.leaky_relu(x)
        x = torch.cat((x, skp1), 1)
        x = self.c22(x)
        x = self.leaky_relu(x)

        x = self.c23(x)

        x1 = self.leaky_relu(x[:, :4, :, :])
        x2 = self.sigmoid(x[:, 4, :, :])

        return torch.cat((x1, x2), 1)

class BackWarp(torch.nn.Module):
    def __init__(self, dim, device):
        super(BackWarp, self).__init__()
        self.width = dim[0]
        self.height = dim[1]
        
        # create a grid (require torch 1.10.2)
        xGrid, yGrid = torch.meshgrid(torch.arange(0, self.width, 1),torch.arange(0, self.height, 1), indexing="xy")
        
        xGrid.requires_grad = False
        xGrid.to(device)
        yGrid.requires_grad = False
        yGrid.to(device)
        
        self.xGrid = torch.clone(xGrid)
        self.yGrid = torch.clone(yGrid)

    def forward(self, F_t, yuvFrame):
        # get xFlow and yFlow tensors
        xFlow_t = F_t[:, 0, :, :]
        yFlow_t = F_t[:, 1, :, :]
        # calculate xGridFlow and yGridFlow
        xGridFlow = self.xGrid.unsqueeze(0).expand_as(xFlow_t).float() + xFlow_t
        yGridFlow = self.yGrid.unsqueeze(0).expand_as(yFlow_t).float() + yFlow_t
        # normalize
        xGridFlow = 2 * (xGridFlow / self.width - 0.5)
        yGridFlow = 2 * (yGridFlow / self.height - 0.5)
        # stack and bilinear interpolation
        yuvFrameGrid = torch.stack((xGridFlow, yGridFlow), dim = 3)
        yuvFrame = torch.nn.functional.grid_sample(yuvFrame, yuvFrameGrid)
        return yuvFrame
