#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "FP16_MAC_algorithim.c"
#include <time.h>

#define mistake_thresh 0.01

float FP162float(unsigned short data);
int verify(unsigned short datak_in,unsigned short datax_in,unsigned short datab_in,FILE * p);
float floatabs(float data);
unsigned short float2FP16(float a);

int main()
{
    unsigned short datak_in,datax_in,datab_in;
    int error_count;
    FILE *p = fopen("report.txt","w");
    int a;

    error_count = 0;                        //错误计数初始化
    datak_in = 0x0000;
    datax_in = 0x3c00;
    datab_in = 0x0000 ;

    while(1)
    {

        while(1)
        {

            // while(1)
            // {

                error_count += verify(datak_in,datax_in,datab_in,p);

                if(error_count >= 10)
                    return 0;

            //     if(datab_in == 0x7bff)
            //         datab_in = 0x8001;
            //     else if(datab_in == 0xfbff)
            //     {
            //         datab_in = 0x0000;
            //         break;
            //     }
            //     else
            //         datab_in +=1;
            // }

            if(datab_in == 0x7bff)
                datab_in = 0x8001;
            else if(datab_in == 0xfbff)
            {
                datab_in = 0x0000;
                break;
            }
            else
                datab_in +=1;

        }

        if(datak_in == 0x7bff)
            datak_in = 0x8001;
        else if(datak_in == 0xfbff)
            break;
        else
            datak_in +=1;
        
        //printf("k=%04x\n",datak_in);
    }

    printf("\nover,there are %d errors\n",error_count);
    fprintf(p,"\nover,there are %d errors\n",error_count);
    fclose(p);

    return 0;
}



//用于验证操作数是否正确并将验证结果写入文件
int verify(unsigned short datak_in,unsigned short datax_in,unsigned short datab_in,FILE * p)
{
    float datak_f = FP162float(datak_in),datax_f = FP162float(datax_in),datab_f = FP162float(datab_in);
    unsigned short data_standard_FP16 = float2FP16(datak_f * datax_f + datab_f);
    float data_standard = FP162float(data_standard_FP16);
    float mistake;
    unsigned short data_my_FP16;
    float data_my_f;
    int a;

    data_my_FP16 = FP16_MAC(datak_in,datax_in,datab_in);
    data_my_f = FP162float(data_my_FP16);

    mistake = floatabs(data_standard - data_my_f);       //计算误差

    if((mistake <= floatabs(data_standard * mistake_thresh)))     //误差在1%以内
    {
        //fprintf(p,"k=%.10f(0x%04x),x=%.10f(0x%04x),b=%.10f(0x%04x),my output is %.10f(0x%04x),it should be %.10f,pass1\n",datak_f,datak_in,datax_f,datax_in,datab_f,datab_in,data_my_f,data_my_FP16,data_standard);
        return 0;
    }
    else if((floatabs(data_standard) > 65504) && ((data_my_FP16 & 0x7fff)== 0x7bff))     //溢出
    {
        //fprintf(p,"k=%.10f(0x%04x),x=%.10f(0x%04x),b=%.10f(0x%04x),my output is %.10f(0x%04x),it should be overflow,pass2\n",datak_f,datak_in,datax_f,datax_in,datab_f,datab_in,data_my_f,data_my_FP16);
        return 0;
    }
    // else if(abs((data_standard_FP16 & 0x7fff) - (data_my_FP16 & 0x7fff)) <= 1)                    //某些不知名原因导致差0x1
    // {
    //     return 0;
    // }
    else                                            //错误
    {
        fprintf(p,"k=%.10f(0x%04x),x=%.10f(0x%04x),b=%.10f(0x%04x),my output is %.10f(0x%04x),it should be %.10f,fail!!!\n",datak_f,datak_in,datax_f,datax_in,datab_f,datab_in,data_my_f,data_my_FP16,data_standard);
        return 1;
    }

}



//将16位short存储的FP16转换成结构相同的float(符号、阶数、尾数相同，而不是值相同)
float FP162float(unsigned short FP16)
{
    float result;
    
    short sign = FP16 >> 15;
    short exp = (FP16 >> 10) & 0x1f;
    short rm = FP16 & 0x03ff;
    if(exp == 0)
    {
        if(rm == 0)
            exp = exp;      //exp = 0,rm = 0,0
        else
            exp = 1;        //exp = 0,rm !=0,denormal
    }
    else
    {
        rm = rm | 0x400;    //exp !=0,normal
    }

    result = rm * pow(2.0,(float)(exp - 25));
    result = sign ? -1*result : result;

    if(exp == 0)      //0x0000 is 0
    {
        //printf("FP162float:0\n");
        return 0;
    }
    else if(exp >= 31)   //0xffff is overflow、NaN
    {
        //printf("FP162float:NaN\n");
        return NAN;
    }
    else
    {
        //printf("FP162float:%f\n",result);
        return result;
    }
}

unsigned short float2FP16(float a)
{
    unsigned short rm;
    bool ifround;
    unsigned char* p = (unsigned char*)&a;
    unsigned short sign = *(p+3) >> 7;

    if(((a > 65504) || (a < -65504)))
    {
        //printf("float2FP16:overflow\n");
        return (sign << 15) | 0x7bff;
    }
    
    if(a == 0)
    {
        //printf("float2FP16:0x0000\n");
        return 0x0000;
    }

    char exp = ((*(p+3) << 1) | (*(p+2) >> 7)) - 112;

    rm = ((*(p+2) & 0x7f) << 3) | (*(p+1) >> 5) | 0x400;

    if(exp >= 1)    //normal
    {
        rm = rm;
        ifround = *(p+1) & 0x10;
    }
    else if(exp >= -10)      //denormal
    {
        switch (exp)
        {
            case 0:{ifround = rm & 0x01;rm = rm >> 1;}break;
            case -1:{ifround = (rm >> 1) & 0x01;rm = rm >> 2;}break;
            case -2:{ifround = (rm >> 2) & 0x01;rm = rm >> 3;}break;
            case -3:{ifround = (rm >> 3) & 0x01;rm = rm >> 4;}break;
            case -4:{ifround = (rm >> 4) & 0x01;rm = rm >> 5;}break;
            case -5:{ifround = (rm >> 5) & 0x01;rm = rm >> 6;}break;
            case -6:{ifround = (rm >> 6) & 0x01;rm = rm >> 7;}break;
            case -7:{ifround = (rm >> 7) & 0x01;rm = rm >> 8;}break;
            case -8:{ifround = (rm >> 8) & 0x01;rm = rm >> 9;}break;
            case -9:{ifround = (rm >> 9) & 0x01;rm = rm >> 10;}break;
            case -10:{ifround = (rm >> 10) & 0x01;rm = rm >> 11;}break;
        }
        exp = 0;
    }
    else
    {
        exp = 0;sign = 0;rm = 0;ifround = 0;
    }

    if(ifround)         //round
        rm += 1;

    if((exp == 0) && ((rm >> 10) == 0x01))  //denormal to normal
    {
        exp = 1;
    }
    else if((rm >> 11) == 0x01)          //carry
    {
        exp +=1;
        rm = rm >> 1;
    }

    unsigned short result = (rm & 0x3ff) | ((exp & 0x1f) << 10) | (sign << 15);

    //printf("float2FP16:0x%x\n",result);


    return result;
}
 

float floatabs(float data)     //c自带的abs只适用于整形
{
    return (data < 0) ? -1*data : data;
}