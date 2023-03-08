#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

unsigned short FP16_MAC(unsigned short k,unsigned short x,unsigned short b);

// int main()
// {
//     unsigned short k = 0x1077,x = 0x1006,b=0x0000;
//     unsigned short result;
//     result = FP16_MAC(k,x,b);
//     return 0;
// }

unsigned short FP16_MAC(unsigned short k,unsigned short x,unsigned short b)
{

    bool overflow;
    unsigned short S_k,M_k,S_x,M_x,S_b,M_b;
    short E_k,E_x,E_b;        //signed

    unsigned short S_mcl;
    short E_mcl;                 //signed
    unsigned long long    M_mcl;    //[22:0]

    unsigned short S_h,shift_bits,suboradd;
    unsigned long long    M_h,M_l;
    short E_h,E_new;

    unsigned short S_new;
    long long M_new;

    short E_new2;
    unsigned short M_new2;
    bool roundflag1,roundflag2;

    unsigned short datao;


    //step1:input check
    if((k & 0x7fff) == 0x7fff)
        overflow = 1;
    else
    {
        overflow = 0;
        S_k = k >> 15;
        E_k = (k >> 10) & 0x1f;
        M_k = k & 0x03ff;
        if(E_k == 0x00)  //if(k[14:10] == 5'b00000)
        {
            if(M_k == 0)                //0
                E_k = E_k;
            else                        //denormal
                E_k = 1;
        }
        else    
            M_k = M_k | 0x400;          //normal
    }

    if((x & 0x7fff) == 0x7fff)
        overflow = 1;
    else
    {
        overflow = 0;
        S_x = x >> 15;
        E_x = (x >> 10) & 0x1f;
        M_x = x & 0x03ff;
        if(E_x == 0x00)
        {
            if(M_x == 0)        //0
                E_x = E_x;
            else                //denormal
                E_x = 1;
        }
        else                    //normal
            M_x = M_x | 0x400;
    }

    if((b & 0x7fff) == 0x7fff)
        overflow = 1;
    else
    {
        overflow = 0;
        S_b = b >> 15;
        E_b = (b >> 10) & 0x1f;
        M_b = b & 0x03ff;
        if(E_b == 0x00)
        {
            if(M_b == 0)        //0
                E_b = E_b;
            else                //denormal
                E_b = 1;     //bias
        }
        else                    //normal
            M_b = M_b | 0x400;
    }
    
    //step2:mcl
    if(overflow)
    {}
    else
    {
        S_mcl = (S_k ^ S_x) ? 1 : 0;
        E_mcl = E_k + E_x - 15;              //real + 30
        M_mcl = M_k * M_x;
    }

    //step3:carry
    if(overflow)
    {}
    else
    {
        if((M_mcl & 0x200000) == 0x200000)    //第22位是1
        {
            M_mcl = M_mcl >> 1;
            E_mcl += 1;
        }
    }

    //mclover

    //step4:compare
    if(overflow)
    {}
    else
    {
        suboradd = (S_mcl == S_b) ? 1 : 0;
        if((E_b == 0) || (E_mcl > E_b))   //((E_b == 0) || (expmcl > expb) )
        {
            S_h = S_mcl;
            E_h = E_mcl;
            M_h = M_mcl << 11;
            M_l = (unsigned long long)M_b << 21;
            shift_bits = E_mcl - E_b;
        }
        else
        {
            S_h = S_b;
            E_h = E_b;
            M_h = (unsigned long long)M_b << 21;
            M_l = M_mcl << 11;
            shift_bits = E_b - E_mcl;
        }
    }

    //step5:shift to align
    if(overflow)
    {}
    else
    {
        switch(shift_bits)
        {
            case 0 : M_l = M_l; break;
            case 1 : M_l = M_l >> 1;    break;
            case 2 : M_l = M_l >> 2;    break;
            case 3 : M_l = M_l >> 3;    break;
            case 4 : M_l = M_l >> 4;    break;
            case 5 : M_l = M_l >> 5;    break;
            case 6 : M_l = M_l >> 6;    break;
            case 7 : M_l = M_l >> 7;    break;
            case 8 : M_l = M_l >> 8;    break;
            case 9 : M_l = M_l >> 9;    break;
            case 10 : M_l = M_l >> 10;    break;
            case 11 : M_l = M_l >> 11;    break;
            case 12 : M_l = M_l >> 12;    break;
            case 13 : M_l = M_l >> 13;    break;
            case 14 : M_l = M_l >> 14;    break;
            case 15 : M_l = M_l >> 15;    break;
            case 16 : M_l = M_l >> 16;    break;
            case 17 : M_l = M_l >> 17;    break;
            case 18 : M_l = M_l >> 18;    break;
            case 19 : M_l = M_l >> 19;    break;
            case 20 : M_l = M_l >> 20;    break;
            case 21 : M_l = M_l >> 21;    break;
            default : M_l = 0;    break;
        }
    }

    //step6:add
    if(overflow)
    {}
    else
    {
        S_new = S_h;
        E_new = E_h;
        if(suboradd)
            M_new = M_h + M_l;
        else
            M_new = M_h - M_l;

    }

    //step8:abs
    if(M_new < 0)               //可能会负，需要取反
    {
        S_new = S_new == 0 ? 1 : 0;
        M_new = M_new * -1;
    }

    //step7:normalization_pre
    if(overflow)
    {}
    else
    {
        if((M_new >= 0x100000000) && (M_new <= 0x1ffffffff))  //1_0000_0000_0000_0000_0000_0000_0000_0000
        {
            E_new2 = E_new + 1;
            M_new2 = M_new >> 22;  //rm_new[32:22]
            roundflag1 = (M_new & 0x200000) == 0x200000;  //M_new[21]
        }
        else if((M_new >= 0x80000000) && (M_new <= 0xffffffff))  //0_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new;
            M_new2 = M_new >> 21;  //M_new[31:21]
            roundflag1 = (M_new & 0x100000) == 0x100000;  //M_new[20]
        }
        else if((M_new >= 0x40000000) && (M_new <= 0x7fffffff))  //0_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 1;
            M_new2 = M_new >> 20;  //M_new[30:20]
            roundflag1 = (M_new & 0x80000) == 0x80000;  //M_new[19]
        }
        else if((M_new >= 0x20000000) && (M_new <= 0x3fffffff))  //0_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 2;
            M_new2 = M_new >> 19;  //M_new[29:19]
            roundflag1 = (M_new & 0x40000) == 0x40000;  //M_new[18]
        }
        else if((M_new >= 0x10000000) && (M_new <= 0x1fffffff))  //0_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 3;
            M_new2 = M_new >> 18;  //M_new[28:18]
            roundflag1 = (M_new & 0x20000) == 0x20000;  //M_new[17]
        }
        else if((M_new >= 0x8000000) && (M_new <= 0xfffffff))  //0_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 4;
            M_new2 = M_new >> 17;  //M_new[27:17]
            roundflag1 = (M_new & 0x10000) == 0x10000;  //M_new[16]
        }
        else if((M_new >= 0x4000000) && (M_new <= 0x7ffffff))  //0_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 5;
            M_new2 = M_new >> 16;  //M_new[26:16]
            roundflag1 = (M_new & 0x8000) == 0x8000;  //M_new[15]
        }
        else if((M_new >= 0x2000000) && (M_new <= 0x3ffffff))  //0_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 6;
            M_new2 = M_new >> 15;  //M_new[25:15]
            roundflag1 = (M_new & 0x4000) == 0x4000;  //M_new[14]
        }
        else if((M_new >= 0x1000000) && (M_new <= 0x1ffffff))  //0_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 7;
            M_new2 = M_new >> 14;  //M_new[24:14]
            roundflag1 = (M_new & 0x2000) == 0x2000;  //M_new[13]
        }
        else if((M_new >= 0x800000) && (M_new <= 0xffffff))  //0_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 8;
            M_new2 = M_new >> 13;  //M_new[23:13]
            roundflag1 = (M_new & 0x1000) == 0x1000;  //M_new[12]
        }
        else if((M_new >= 0x400000) && (M_new <= 0x7fffff))  //0_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 9;
            M_new2 = M_new >> 12;  //M_new[22:12]
            roundflag1 = (M_new & 0x800) == 0x800;  //M_new[11]
        }
        else if((M_new >= 0x200000) && (M_new <= 0x3fffff))  //0_0000_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 10;
            M_new2 = M_new >> 11;  //M_new[21:11]
            roundflag1 = (M_new & 0x400) == 0x400;  //M_new[10]
        }
        else if((M_new >= 0x100000) && (M_new <= 0x1fffff))  //0_0000_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 11;
            M_new2 = M_new >> 10;  //M_new[20:10]
            roundflag1 = (M_new & 0x200) == 0x200;  //M_new[9]
        }
        else if((M_new >= 0x80000) && (M_new <= 0xfffff))  //0_0000_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 12;
            M_new2 = M_new >> 9;  //M_new[19:9]
            roundflag1 = (M_new & 0x100) == 0x100;  //M_new[8]
        }
        else if((M_new >= 0x40000) && (M_new <= 0x7ffff))  //0_0000_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 13;
            M_new2 = M_new >> 8;  //M_new[18:8]
            roundflag1 = (M_new & 0x80) == 0x80;  //M_new[7]
        }
        else if((M_new >= 0x20000) && (M_new <= 0x3ffff))  //0_0000_0000_0000_001x_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 14;
            M_new2 = M_new >> 7;  //M_new[17:7]
            roundflag1 = (M_new & 0x40) == 0x40;  //M_new[6]
        }
        else if((M_new >= 0x10000) && (M_new <= 0x1ffff))  //0_0000_0000_0000_0001_xxxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 15;
            M_new2 = M_new >> 6;  //M_new[16:6]
            roundflag1 = (M_new & 0x20) == 0x20;  //M_new[5]
        }
        else if((M_new >= 0x8000) && (M_new <= 0xffff))  //0_0000_0000_0000_0000_1xxx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 16;
            M_new2 = M_new >> 5;  //M_new[15:5]
            roundflag1 = (M_new & 0x10) == 0x10;  //M_new[4]
        }
        else if((M_new >= 0x4000) && (M_new <= 0x7fff))  //0_0000_0000_0000_0000_01xx_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 17;
            M_new2 = M_new >> 4;  //M_new[14:4]
            roundflag1 = (M_new & 0x8) == 0x8;  //M_new[3]
        }
        else if((M_new >= 0x2000) && (M_new <= 0x3fff))  //0_0000_0000_0000_0000_001x_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 18;
            M_new2 = M_new >> 3;  //M_new[13:3]
            roundflag1 = (M_new & 0x4) == 0x4;  //M_new[2]
        }
        else if((M_new >= 0x1000) && (M_new <= 0x1fff))  //0_0000_0000_0000_0000_0001_xxxx_xxxx_xxxx
        {
            E_new2 = E_new - 19;
            M_new2 = M_new >> 2;  //M_new[12:2]
            roundflag1 = (M_new & 0x2) == 0x2;  //M_new[1]
        }
        else if((M_new >= 0x800) && (M_new <= 0xfff))  //0_0000_0000_0000_0000_000_1xxx_xxxx_xxxx
        {
            E_new2 = E_new - 20;
            M_new2 = M_new >> 1;  //M_new[11:1]
            roundflag1 = (M_new & 0x1) == 0x1;  //M_new[0]
        }
        else if((M_new >= 0x400) && (M_new <= 0x7ff))  //0_0000_0000_0000_0000_000_01xx_xxxx_xxxx
        {
            E_new2 = E_new - 21;
            M_new2 = M_new;  //M_new[10:0]
            roundflag1 = 0;
        }
        else if((M_new >= 0x200) && (M_new <= 0x3ff))  //0_0000_0000_0000_0000_000_001x_xxxx_xxxx           //new
        {
            E_new2 = E_new - 22;
            M_new2 = M_new << 1;  //M_new[9:0],0
            roundflag1 = 0;
        }
        else if((M_new >= 0x100) && (M_new <= 0x1ff))  //0_0000_0000_0000_0000_000_0001_xxxx_xxxx           //new
        {
            E_new2 = E_new - 23;
            M_new2 = M_new << 2;  //M_new[8:0],00
            roundflag1 = 0;
        }
        else if((M_new >= 0x80) && (M_new <= 0xff))  //0_0000_0000_0000_0000_000_0000_1xxx_xxxx           //new
        {
            E_new2 = E_new - 24;
            M_new2 = M_new << 3;  //M_new[7:0],000
            roundflag1 = 0;
        }
        // else if((M_new >= 0x40) && (M_new <= 0x7f))  //0_0000_0000_0000_0000_000_0000_01xx_xxxx           //new
        // {
        //     E_new2 = E_new - 25;
        //     M_new2 = M_new << 4;  //M_new[6:0],0000
        //     roundflag = 0;
        // }
        // else if((M_new >= 0x20) && (M_new <= 0x3f))  //0_0000_0000_0000_0000_000_0000_001x_xxxx           //new
        // {
        //     E_new2 = E_new - 26;
        //     M_new2 = M_new << 5;  //M_new[5:0],00000
        //     roundflag = 0;
        // }
        // else if((M_new >= 0x10) && (M_new <= 0x1f))  //0_0000_0000_0000_0000_000_0000_0001_xxxx           //new
        // {
        //     E_new2 = E_new - 27;
        //     M_new2 = M_new << 6;  //M_new[4:0],000000
        //     roundflag = 0;
        // }
        else
        {
            E_new2 = 0;
            M_new2 = 0;
            roundflag1 = 0;
        }
    }


    //step8:carry2
    if(overflow)
    {}
    else
    {
        if(E_new2 <1)       //denormal
        {
            switch (E_new2)
            {
                case 0:{roundflag2 = M_new2 & 0x1;M_new2 = M_new2 >> 1;}break;
                case -1:{roundflag2 = (M_new2 >> 1) & 0x01;M_new2 = M_new2 >> 2;}break;
                case -2:{roundflag2 = (M_new2 >> 2) & 0x01;M_new2 = M_new2 >> 3;}break;
                case -3:{roundflag2 = (M_new2 >> 3) & 0x01;M_new2 = M_new2 >> 4;}break;
                case -4:{roundflag2 = (M_new2 >> 4) & 0x01;M_new2 = M_new2 >> 5;}break;
                case -5:{roundflag2 = (M_new2 >> 5) & 0x01;M_new2 = M_new2 >> 6;}break;
                case -6:{roundflag2 = (M_new2 >> 6) & 0x01;M_new2 = M_new2 >> 7;}break;
                case -7:{roundflag2 = (M_new2 >> 7) & 0x01;M_new2 = M_new2 >> 8;}break;
                case -8:{roundflag2 = (M_new2 >> 8) & 0x01;M_new2 = M_new2 >> 9;}break;
                case -9:{roundflag2 = (M_new2 >> 9) & 0x01;M_new2 = M_new2 >> 10;}break; 
                case -10:{roundflag2 = (M_new2 >> 10) & 0x01;M_new2 = M_new2 >> 11;}break;     
                default:{roundflag2 = 0;M_new2 = 0;}break;
            }
            M_new2 = roundflag2 ? M_new2 + 1 : M_new2;
            if(M_new2 >> 10)        //denormal to normal
                E_new2 = 1;
            else
                E_new2 = 0;
        }
        else            //normal
        {
            M_new2 = roundflag1 ? M_new2 +1 : M_new2;
            if((M_new2 & 0x800) == 0x800)  //rm_new2[11]
            {
                M_new2 = M_new2 >> 1;
                E_new2 +=1 ;
            }
        }

    }


    //step9:output
    if(overflow || (E_new2 > 30))
        datao = (S_new << 15) | 0x7bff;
    else if(M_new2 == 0)
        datao = 0x0000;
    else
        datao = (S_new << 15) | (E_new2 << 10) | (M_new2 & 0x3ff);

    return datao;
}