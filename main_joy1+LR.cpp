// ジョイスティック１本＋LR
#include "mbed.h"
#include "QEI.h"
#include "PS3.h"
#include <cstdint>  //uint8 (char) を使用したら勝手に追加された

#define M_R_WHEEL 0x66
#define M_L_WHEEL 0x64
#define M3ADDRESS 0x62
#define M4ADDRESS 0x60

Thread th1;
Thread th2;
int thread1();
int thread2();

I2C i2c(D14,D15); // D14 = PB_8, D15 = PB_9
PS3 ps3( A0, A1); // PA_0, PA_1

DigitalIn button(BUTTON1);
DigitalIn sw1(PC_4);
DigitalIn sw2(PB_15);
DigitalIn sw3(PB_12);
DigitalIn sw4(PB_15);
DigitalIn sw5(PC_8);

DigitalOut myled(LED1);
DigitalOut led1(PA_9);  //  PA_9 = D8
DigitalOut led2(PA_8);  //  PA_8 = D7
DigitalOut led3(PB_10); // PB_10 = D6
DigitalOut led4(PB_4);  //  PB_4 = D5
DigitalOut led5(PB_5);  //  PB_5 = D4

QEI rori1(  PD_2, PC_11, NC, 2048, QEI::X2_ENCODING);
QEI rori2( PC_10, PC_12, NC, 2048, QEI::X2_ENCODING);
QEI rori3( PA_13, PA_14, NC, 2048, QEI::X2_ENCODING);
QEI rori4( PA_15,  PB_7, NC, 2048, QEI::X2_ENCODING);
QEI rori5(  PH_0,  PH_1, NC, 2048, QEI::X2_ENCODING);
QEI rori6(  PC_3,  PA_4, NC, 2048, QEI::X2_ENCODING);

void send(char, char);
int outputWheelData(char *L_data, char *R_data);

int pulse1, pulse2, pulse3, pulse4, pulse5, pulse6;
float angle1, angle2, angle3, angle4, angle5, angle6;

char R_W_Mdata, L_W_Mdata; //足回りのMDに送るPWMデータ

int main (void){

    R_W_Mdata = 0x80;
    L_W_Mdata = 0x80;

    th1.start(thread1);
    th2.start(thread2);

    rori1.reset();
    rori2.reset();
    rori3.reset();
    rori4.reset();
    rori5.reset();
    rori6.reset();

    while (true){
        outputWheelData(&L_W_Mdata, &R_W_Mdata);
        send(M_R_WHEEL, 256 - L_W_Mdata);
        send(M_L_WHEEL, R_W_Mdata);
        send(M3ADDRESS, 0x80);
        send(M4ADDRESS, 0x80);
    }

}

int thread1 (void){
    while (true){
        // ps3.printdata();
        // printf("lx:%2d ly:%2d la:%3.1f rx:%2d ry:%2d ra:%3.1f\n", ps3.getLeftJoystickXaxis(), ps3.getLeftJoystickYaxis(), ps3.getLeftJoystickAngle(), ps3.getRightJoystickXaxis(), ps3.getRightJoystickYaxis(), ps3.getRightJoystickAngle());
        printf("Ly:%2d Ldata16:%2x Ldata10:%3d Ry:%2d Rdata16:%2x Rdata10:%3d \n", ps3.getLeftJoystickYaxis(), L_W_Mdata, L_W_Mdata, ps3.getRightJoystickYaxis(),R_W_Mdata, R_W_Mdata);
        // printf("%d\n",Lx);
        ThisThread::sleep_for(100ms);
    }
}

int thread2 (void){
    while (true){
        pulse1 = rori1.getPulses();
        pulse2 = rori2.getPulses();
        pulse3 = rori3.getPulses();
        pulse4 = rori4.getPulses();
        pulse5 = rori5.getPulses();
        pulse6 = rori6.getPulses();
        
        angle1 = ( 360*(float)pulse1 / (2048*2) );
        angle2 = ( 360*(float)pulse2 / (2048*2) );
        angle3 = ( 360*(float)pulse3 / (2048*2) );
        angle4 = ( 360*(float)pulse4 / (2048*2) );
        angle5 = ( 360*(float)pulse5 / (2048*2) );
        angle6 = ( 360*(float)pulse6 / (2048*2) );
    }
}

void send (char address, char data){
    i2c.start();
    i2c.write(address);
    i2c.write(data);
    i2c.stop();
    ThisThread::sleep_for(15ms);
}


int outputWheelData (char *L_data, char *R_data){
    int Ry;
    *L_data = 128; *R_data = 128; //break

    if ( ps3.getRightJoystickYaxis() == 64){ //右スティックで出力調整、charは0~255までの値しかとらないので64-1で調整
        Ry = 64-1;
    }else{
        Ry = ps3.getRightJoystickYaxis();
    }

    if( ps3.getButtonState(PS3::L1) && Ry == 0){ //超信地旋回ccw 値要調整
        *L_data -= 30;
        *R_data += 30;
    } else if ( ps3.getButtonState(PS3::L1) && Ry > 0){ //ccw旋回
        *L_data -= 30;
    } else if ( ps3.getButtonState(PS3::L1) && Ry < 0){
        *L_data += 30;
    }

    if( ps3.getButtonState(PS3::R1) && Ry == 0){ //超信地旋回cw 値要調整
        *L_data += 30;
        *R_data -= 30;
    } else if ( ps3.getButtonState(PS3::R1) && Ry > 0){ //cw旋回
        *R_data -= 30;
    } else if ( ps3.getButtonState(PS3::R1) && Ry < 0){
        *R_data += 30;
    }

    *L_data += Ry*2;
    *R_data += Ry*2;

    return 0;
}
