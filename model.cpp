#include <torch/torch.h>

// TODO: AvgPool2dFuncOptions is a little suspicious with the kernel size parameter
// TODO: check bilinear upscaling factor code
// TODO: check whether grids work correctly in BackWarp

/**
 * @brief Struct to create Encoder Hierarchy Layers
 * Avg Pooling Layer -> Convolutional Layer + Leaky ReLu -> Convolutional Layer + Leaky ReLu
 */
struct EncoderHierarchy : torch::nn::Module {
    EncoderHierarchy(int inChannels, int outChannels, int kernelSize)
        : conv1(torch::nn::ConvTranspose2dOptions(inChannels, outChannels, kernelSize)
                .stride(1)
                .padding((int)((kernelSize - 1) / 2))),
        conv2(torch::nn::ConvTranspose2dOptions(outChannels, outChannels, kernelSize)
                .stride(1)
                .padding((int)((kernelSize - 1) / 2)))
    {
        register_module("conv1", conv1);
        register_module("conv2", conv2);
    }

    torch::Tensor forward(torch::Tensor x) {
        namespace F = torch::nn::functional;
        x = F::avg_pool2d(x, F::AvgPool2dFuncOptions(2).stride(2));
        F::leaky_relu(conv1(x), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        F::leaky_relu(conv2(x), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        return x;
    }

    torch::nn::ConvTranspose2d conv1, conv2;
};

/**
 * @brief Struct to create Decoder Hierarchy Layers
 * Bilinear interpolation -> Convolutional Layer + Leaky ReLu -> Convolutional Layer + Leaky ReLu
 */
struct DecoderHierarchy : torch::nn::Module {
    DecoderHierarchy(int inChannels, int outChannels, int kernelSize)
        : conv1(torch::nn::ConvTranspose2dOptions(inChannels, outChannels, kernelSize)
                .stride(1)
                .padding((int)((kernelSize - 1) / 2))),
        conv2(torch::nn::ConvTranspose2dOptions(2 * outChannels, outChannels, kernelSize)
                .stride(1)
                .padding((int)((kernelSize - 1) / 2)))
    {
        register_module("conv1", conv1);
        register_module("conv2", conv2);
    }

    torch::Tensor forward(torch::Tensor x, torch::Tensor skp) {
        namespace F = torch::nn::functional;
        x = F::interpolate(x, F::InterpolateFuncOptions().scale_factor(std::vector<double>({2})).mode(torch::kBilinear));
        F::leaky_relu(conv1(torch::cat({x, skp}, 1)), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        F::leaky_relu(conv2(x), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        return x;
    }

    torch::nn::ConvTranspose2d conv1, conv2;
};

/**
 * @brief Struct to create a UNet Architecture Model for arbitrary time frame interpolation
 */
struct UNet : torch::nn::Module {
    UNet()
        // Use 7x7 filter in the first two convolutional layers
        : conv1(torch::nn::ConvTranspose2dOptions(20, 32, 7)
                .stride(1)
                .padding(3)),
        conv2(torch::nn::ConvTranspose2dOptions(32, 32, 7)
                .stride(1)
                .padding(3)),
        // 5x5 filter in the second hierarchy
        eh1(32, 64, 5),
        // 3x3 filter in the rest
        eh2(64, 128, 3),
        eh3(128, 256, 3),
        eh4(256, 512, 3),
        eh5(512, 512, 3),
        dh1(512, 512, 3),
        dh2(512, 256, 3),
        dh3(256, 128, 3),
        dh4(128, 64, 3),
        dh5(64, 32, 3),
        conv3(torch::nn::ConvTranspose2dOptions(32, 5, 3)
                .stride(1)
                .padding(1))
    {
        register_module("conv1", conv1);
        register_module("conv2", conv2);
        register_module("conv3", conv3);
    }

    torch::Tensor forward(torch::Tensor x) {
        namespace F = torch::nn::functional;
        // save skip connections
        torch::Tensor skp1 = F::leaky_relu(conv1(x), F::LeakyReLUFuncOptions().negative_slope(0.1));
        F::leaky_relu(conv2(skp1), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        torch::Tensor skp2 = eh1.forward(skp1);
        torch::Tensor skp3 = eh2.forward(skp2);
        torch::Tensor skp4 = eh3.forward(skp3);
        torch::Tensor skp5 = eh4.forward(skp4);
        // call forward methods with skip connections
        x = eh5.forward(skp5);
        x = dh1.forward(x, skp5);
        x = dh2.forward(x, skp4);
        x = dh3.forward(x, skp3);
        x = dh4.forward(x, skp2);
        x = dh5.forward(x, skp1);
        F::leaky_relu(conv3(x), F::LeakyReLUFuncOptions().negative_slope(0.1).inplace(true));
        return x;
    }
    
    torch::nn::ConvTranspose2d conv1, conv2, conv3;
    EncoderHierarchy eh1, eh2, eh3, eh4, eh5;
    DecoderHierarchy dh1, dh2, dh3, dh4, dh5;
};

struct BackWarp : torch::nn::Module {
    BackWarp(int width, int height) {
        this->width = width;
        this->height = height;
        torch::Tensor x = torch::arange(0, this->width);
        torch::Tensor y = torch::arange(0, this->height);
        std::vector<torch::Tensor> grids = torch::meshgrid({x, y});
        xGrid = grids[0];
        yGrid = grids[1];
    }

    // take yuvFrame as an argument as well
    torch::Tensor forward(torch::Tensor F_t) {
        namespace F = torch::nn::functional;
        // add xGrid to xFlow (need to unsqueeze to add outer dim)
        // add yGrid to yFlow (need to unsqueeze to add outer dim)
            // also might need to call expand as to be safe
        torch::Tensor xGridFlow;
        torch::Tensor yGridFlow;

        // TODO: convert this python to C++
        // xGridFlow = 2 * (x / width - 0.5)
        // yGridFlow = 2 * (y / height - 0.5)

        torch::Tensor yuvFrameGrid = torch::stack({xGridFlow, yGridFlow}, 3);
        
        // TODO: finish with yuv frame
        // return F::grid_sample(yuvFrame, yuvFrameGrid);
    }

    int width, height;
    torch::Tensor xGrid, yGrid;
};

// TODO: training code
