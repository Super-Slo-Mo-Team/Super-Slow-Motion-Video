
import subprocess
import os

CUDA_COMMAND = "C:\\Users\\SeanH\\Documents\\ecs193\\Optical_Flow_SDK_3.0.15 - Copy\\NvOFBasicSamples\\build\\AppOFCuda\\Debug\\AppOFCuda.exe"

def getFlow(directory):
    # loop through each folder in directory
        # create output folder
        # pass frames to app of cuda with specified output folder

    for root, dirs, files in os.walk(directory, topdown=True):
        for name in dirs:
            # create output
            output_dir = root + "/" + name + "_out"
            os.makedirs(output_dir)
            # run app of cuda with output
            subprocess.run([CUDA_COMMAND, f'--input={directory}\\{name}\\*.yuv', f"--output={output_dir}/"])
 

def main():
    getFlow("C:\\Users\\SeanH\\Documents\\ecs193\\akiyo_test_4")

if __name__ == "__main__":
    main()

        