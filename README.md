# 基于AlexeyAB/darknet更改
### 远程画图
* 编译需要先安装`sudo apt-get install libcurl4-openssl-dev`
* `voc.data`新增参数
    * draw_url=http://192.168.31.75:18888/draw_chart
    * project_id=项目名称
### 加密说明
power-ai.dll是秘钥文件，每次发人demo需要替换掉power-ai.dll，只要demo运行一次就会写入`power-ai.dll`90天的时间限制和硬件唯一标示，下次运行会判断
config.data 为 darknet的加密配置文件，每次运行会解密在`C:\Program Files\Windows Config\winD.dll`，程序会自动调用
配置文件加密秘钥为zengyining的MD5
秘钥文件加密秘钥为baymin1024的MD5

### 加密数据说明
https://github.com/yiningzeng/darknet-license/blob/master/src/parser.c#L1588-L1590

## 说明
提供了c++和c# 
 * c++ demo演示了如何使用动态库来检测一个目录下所有的图片
 * c# 版本 已经集成opencv相关的操作在dll中，可直接调用 

### 报错说明
>CUDA status Error: file: …\src\dark_cuda.c : cuda_set_device() : line: 36 
CUDA Error: unknown error
显卡驱动版本太低了，更新到最新版本即可

> failed to load cfg 请使用管理员权限运行编译器

## 目录
```
├─test
│  │  test.cpp
│  │  test.vcxproj
│  │  test.vcxproj.filters
│  │  test.vcxproj.user
│  ├─include
│  │      ai_cpp_dll.lib
│  │      ai_v2_class.hpp
└─x64
    └─Release
        │  ai_cpp_dll.dll
        │  ai_cpp_dll.lib
        │  opencv_world410.dll
        │  pthreadGC2.dll
        │  pthreadVC2.dll
        │  test.exe
        └─yunsheng
            ├─data 
            │      ai-voc_last.weights #和power-ai.dll一定要在同一目录
            │      config.data 
            │      power-ai.dll #和ai-voc_last.weights一定要在同一目录
            │      voc.names
            ├─result
            └─testimg
```
## C++ demo说明
---
### 参数说明
* thresh 检测的阈值
* names_file 对应的缺陷分类对象名称文件(缺陷名可更改)
* cfg_file 配置文件路径
* weights_file 权重文件

```c++
	float thresh = 0.2;//设置阈值
	string  names_file = "C:/Users/Administrator/source/repos/test/x64/Release/yunsheng/data/voc.names";
	string  cfg_file = "C:/Users/Administrator/source/repos/test/x64/Release/yunsheng/data/config.data";
	string  weights_file = "C:/Users/Administrator/source/repos/test/x64/Release/yunsheng/data/ai-voc_last.weights";
```
### demo中使用了opencv来处理图片和描框，这里不做配置说明，主要说明如何接入检测
1. 引用`ai_cpp_dll.lib`，配置项目>属性>(选择`Release`)>链接库>附加目录库填写`ai_cpp_dll.lib`所在的目录
![图片说明](https://upload-images.jianshu.io/upload_images/6639127-44ebc9a032e8fa87.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
2. 链接库
```c++
#pragma comment(lib, "ai_cpp_dll.lib")
```
3. 引入`ai_v2_class.hpp`头文件
```c++
#include "ai_v2_class.hpp"
```
4. 初始化检测器
```c++
Detector detector(cfg_file, weights_file);
```
5. 通过`objects_names_from_file`方法获得分类对象名称
```c++
auto obj_names = objects_names_from_file(names_file);
```
6. 读取文件夹下图片检测
    检测的函数`detector.detect(mat_img, thresh)`
```c++
		cout << ">>>>>>>>>>>>>>>>>>>>>>>>>> \n" << files[i].c_str()<<endl;
		cv::Mat mat_img = cv::imread(files[i].c_str()); // opencv 读取图片
		auto start = std::chrono::steady_clock::now(); // 开始时间
		std::vector<bbox_t> result_vec = detector.detect(mat_img, thresh); // 检测函数
		auto end = std::chrono::steady_clock::now(); // 结束时间
		std::chrono::duration<double> spent = end - start; // 检测时间
		std::cout << " Time: " << spent.count() << " sec \n<<<<<<<<<<<<<<<<<<<<<<<<\n";
		draw_boxes(mat_img, result_vec, obj_names); // 最后把缺陷框描绘在图像上
		cv::imwrite("C:/Users/Administrator/source/repos/test/x64/Release/yunsheng/result/" + filesname[i], mat_img); // 保存检测的结果图片
```
### 

## C# demo说明
---
主要定义在AITtestSDK.cs
```c#
using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace ConsoleApp2
{
    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bbox_t
    {
        public uint x, y, w, h;       // 缺陷框坐标 定点x,y 宽高w,h
        public float prob;            // 置信度
        public uint obj_id;           // 缺陷id
        public uint track_id;         // 预留，tracking id for video (0 - untracked, 1 - inf - tracked object)
        public uint frames_counter;   // 预留，counter of frames on which the object was detected
        public float x_3d, y_3d, z_3d;// 预留 

    };
    [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct bbox_t_container
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
        public bbox_t[] bboxlist;
    };
    public class AITestSDK
    {
        /// <summary>
        /// 初始化
        /// </summary>
        /// <param name="configurationFilename">配置文件路径</param>
        /// <param name="weightsFilename">权重文件路径，这里注意一定要使用斜杠，不能使用反斜杠</param>
        /// <param name="gpuID">gpuid，不清楚的直接填写0，如果要更改请查阅nvidia-smi</param>
        /// <returns></returns>
        [DllImport(@"ai_cpp_dll.dll", EntryPoint = "init", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int init(string configurationFilename, string weightsFilename, int gpuID);

        /// <summary>
        /// 通过byte[]来检测
        /// </summary>
        /// <param name="data">图片byte[]</param>
        /// <param name="data_length">长度</param>
        /// <param name="bbox_T_Container">返回结果</param>
        /// <returns>返回-1表示，调用opencv失败</returns>
        [DllImport(@"ai_cpp_dll.dll", EntryPoint = "detect_mat", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int detect_opencv_mat(byte[] data, long data_length, ref bbox_t_container bbox_T_Container);

        /// <summary>
        /// 通过图片路径检测
        /// </summary>
        /// <param name="filename">图片路径</param>
        /// <param name="bbox_T_Container">返回结果</param>
        /// <returns></returns>
        [DllImport(@"ai_cpp_dll.dll", EntryPoint = "detect_image", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int detect_image_path(string filename, ref bbox_t_container bbox_T_Container);

        /// <summary>
        /// 释放
        /// </summary>
        /// <returns></returns>
        [DllImport(@"ai_cpp_dll.dll", EntryPoint = "dispose", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int dispose();
    }
}
```
具体使用说明
```c#
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace ConsoleApp2
{
    class Program
    {
        public static byte[] getImageByte(String path)
        {
            FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read); //将图片以文件流的形式进行保存
            BinaryReader br = new BinaryReader(fs);
            byte[] imgBytesIn = br.ReadBytes((int)fs.Length); //将流读入到字节数组中
            return imgBytesIn;
        }

        public static void showConsole(bbox_t_container boxlist)
        {
            Console.WriteLine("\n***************************");
            for (int i = 0; i < boxlist.bboxlist.Length; i++)
            {
                if (boxlist.bboxlist[i].h == 0)
                {
                    break;
                }
                else
                {
                    Console.WriteLine(String.Format("x:{0} y:{1} w:{2} h:{3} prob:{4} obj_id:{5}",
                        boxlist.bboxlist[i].x.ToString(),
                        boxlist.bboxlist[i].y.ToString(),
                        boxlist.bboxlist[i].w.ToString(),
                        boxlist.bboxlist[i].h.ToString(),
                        boxlist.bboxlist[i].prob.ToString(),
                        boxlist.bboxlist[i].obj_id.ToString()
                        ));
                }
            }
            Console.WriteLine("\n***************************");
        }

        static void Main(string[] args)
        {
            bbox_t_container boxlist = new bbox_t_container();
            // VS要用管理员权限打开
            // VS要用管理员权限打开
            // VS要用管理员权限打开

            //初始化检测器
            AITestSDK.init(@"C:\Users\Administrator\source\repos\ConsoleApp2\ConsoleApp2\bin\x64\Debug\netcoreapp2.1\yunsheng\data\config.data",
                // 特别说明，这里的路径一定要用 '/' 不能用反斜杠
                // 特别说明，这里的路径一定要用 '/' 不能用反斜杠
                // 特别说明，这里的路径一定要用 '/' 不能用反斜杠
                "C:/Users/Administrator/source/repos/ConsoleApp2/ConsoleApp2/bin/x64/Debug/netcoreapp2.1/yunsheng/data/ai-voc_last.weights", 
                0);

            string fileName = @"C:\Users\Administrator\source\repos\ConsoleApp2\ConsoleApp2\bin\x64\Debug\netcoreapp2.1\yunsheng\testimg\1.jpg";

            #region 通过文件路径检测
            AITestSDK.detect_image_path(fileName, ref boxlist);
            showConsole(boxlist);
            #endregion

            #region 通过图片byte[]检测
            boxlist = new bbox_t_container();
            byte[] byteImg = getImageByte(fileName);
            int n = AITestSDK.detect_opencv_mat(byteImg, byteImg.Length, ref boxlist);
            if (n == -1) Console.WriteLine("调用失败，请检测目录是否包含opencv的dll");
            showConsole(boxlist);
            #endregion

        }
    }
}
```
