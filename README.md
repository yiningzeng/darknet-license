### 加密说明
power-ai.dll是秘钥文件，每次发人demo需要替换掉power-ai.dll，只要demo运行一次就会写入`power-ai.dll`90天的时间限制和硬件唯一标示，下次运行会判断
config.data 为 darknet的加密配置文件，每次运行会解密在`C:\Program Files\Windows Config\winD.dll`，程序会自动调用
配置文件加密秘钥为zengyining的MD5
秘钥文件加密秘钥为baymin1024的MD5
### 说明
demo演示了如何使用动态库来检测一个目录下所有的图片
### 目录
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
            │      power-ai.dll #和ai-voc_last.weights一定要在同一目录 没次给人demo需要用(power-ai.dll(初始的秘钥验证文件，伪装成dll))替换掉
            │      voc.names
            ├─result
            └─testimg

```
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