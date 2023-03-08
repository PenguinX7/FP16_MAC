#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

unsigned short float2FP16(float a);
float FP162float(unsigned short FP16);

int main()
{
    unsigned short a=float2FP16(3.55271368e-15);    //0x0001
    float b = FP162float(0x0000);
    return 0;
}

unsigned short float2FP16(float a)
{
    unsigned short rm;
    bool ifround;
    if(((a > 65504) || (a < -65504)))
    {
        printf("float2FP16:overflow\n");
        return 0xffff;
    }
    
    if(a == 0)
    {
        printf("float2FP16:0x0000\n");
        return 0x0000;
    }

    unsigned char* p = (unsigned char*)&a;
    
    // printf("%02x    ",*(p+3));
    // printf("%02x",*(p+2));
    // printf("%02x",*(p+1));
    // printf("%02x\n",*(p));

    char exp = ((*(p+3) << 1) | (*(p+2) >> 7)) - 112;

    unsigned short sign = *(p+3) >> 7;

    rm = ((*(p+2) & 0x7f) << 3) | (*(p+1) >> 5) | 0x400;

    if(exp >= 1)    //normal
    {
        rm = rm;
        ifround = *(p+1) & 0x10;
    }
    else if(exp >= -9)      //denormal
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
            case -10:{ifround = (rm >> 10)& 0x01;rm = rm >> 11;}break;
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
        exp = 1;
    if((rm >> 11) == 0x01)          //carry
    {
        exp +=1;
        rm = rm >> 1;
    }

    unsigned short result = (rm & 0x3ff) | ((exp & 0x1f) << 10) | (sign << 15);

    printf("float2FP16:0x%x\n",result);


    return result;
}

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
    // short exp = ((FP16 & 0x7c00) >> 10) - 25;
    // short rm = (FP16 & 0x03ff | 0x0400);

    result = rm * pow(2.0,(float)exp - 25);
    result = sign ? -1*result : result;

    if(exp == 0)      //0x0000 is 0
    {
        printf("FP162float:0\n");
        return 0;
    }
    else if(exp >= 31)   //0xffff is overflow、NaN
    {
        printf("FP162float:NaN\n");
        return NAN;
    }
    else
    {
        printf("FP162float:%f\n",result);
        return result;
    }
}