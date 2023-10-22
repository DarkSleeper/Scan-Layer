<img src="https://github.com/DarkSleeper/Scan-Layer/assets/48831197/ba802d9d-de5f-4e27-b645-c83a3842629d" width="512"><br><br>
扫描线z-buffer算法：<br>
![scan](https://github.com/DarkSleeper/Scan-Layer/assets/48831197/81a7adfe-2790-4046-8d1b-65129d07f1c2)<br><br>
简单层次z-buffer算法：<br>
![simple-layer](https://github.com/DarkSleeper/Scan-Layer/assets/48831197/025f039f-aa4a-4a3b-b1f6-fe7ec4bb9cfe)<br><br>
完整层次z-buffer算法：<br>
![complete-layer](https://github.com/DarkSleeper/Scan-Layer/assets/48831197/c054da60-e990-4ada-8540-0ec58524e1f7)<br><br>

1. 编程环境：<br>
Windows10 + Visual Studio 2019版(16.11.7)

2. 用户界面使用说明：<br>
本作业实现了三种算法：扫描线z-buffer算法，层次z-buffer算法的简单模式(无场景八叉树)以及其完整模式。<br>
上交的作业包含两个文件夹：分别存放工程文件，和供用户使用的源码及执行文件。下面解释如何运行执行文件：<br>
每种算法的文件夹内有exe和source两个文件夹，用户需在终端打开exe文件夹，运行exe文件即可。<br>

3. 运行帮助：<br>
运行命令示范：<br>
xxx\exe> .\Scan-Line_Z-Buffer.exe<br>
xxx\exe> .\Scan-Line_Z-Buffer.exe robot <br><br>
模型文件说明：<br>
用户可以传入一个参数，表示加载"./runtime/model/[argv1].obj"这个模型，默认参数为"bunny"；<br>
支持obj场景导入，场景是通过blender搭建并以三角形面片这一模式导出的；<br>
模型的颜色由模型上各点的法线向量决定。<br><br>
使用说明：<br>
用户运行程序后，可以通过"wasd"进行场景漫游，按住鼠标左键可以以第一人称改变视角；<br>
每帧的CPU耗时（即算法运行时间）显示在窗口左上角；<br>
按"esc"键退出。<br><br>
工程文件说明：<br>
工程文件中的代码是测试代码，运行后相机会围绕物体拍摄一周，并返回平均每帧的CPU耗时。<br>

4. 数据结构说明<br>
见实验报告<br>

5. 加速情况<br>
场景的投影，剔除以及光栅化完全运行在CPU上，使用自己编写的函数；<br>
光栅化生成的帧缓存使用glfw相关函数通过GPU渲染到窗口上；<br>
没有对与窗口边界相交的三角形进行裁切处理，而是直接剔除；<br>
层次z-buffer中，没有每渲染一个点就更新它，<br>
而是在下一帧一起更新层次z-buffer，因为前者在CPU上的表现很差（完整模式的mipmap.cpp中有相关代码）。<br>
