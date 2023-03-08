该工程实现两个FP16（支持规格化和非规格化）的乘累加(kx+b)，输出FP16规格化数或非规格化数

.v和_tb.v是v文件及其测试文件
.v模块端口功能如下
data_k		操作数k
data_x		操作数x
data_b		操作数b
clk		时钟，上升沿触发
rst		复位信号，高电平有效
input_valid	输入有效信号，高电平有效
opcode		操作码，方便例化时进行正在运算的判断，无实际意义
data_o		输出结果
output_update	输出有效信号，高电平有效
opcode_o		操作码，方便例化时进行正在运算的判断，无实际意义

_algorithim.c是算法的C语言实现，方便理解算法并提供测试雏形
_algorithim.docx是关于_algorithim.c的文档解释

_test.c是算法层软件测试文件，调用_algorithim.c中的算法函数进行验证
report.txt是验证报告，只输出错误信息，所以report中什么也没有才是正常的

else.c为FP16形式(unsigned short)与float互相转换的函数的文件，目标MAC算法中未调用

由于三输入难以穷尽，已验证的覆盖如下：
1）k从0x0000~0x000f，x和b穷尽所有合法输入
2）加法验证：x=0x3c00，k和b穷尽所有合法输入
3）乘法验证：b=0x0000，k和x穷尽所有合法输入