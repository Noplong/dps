/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : GPRS.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��5��
***************************************************************/

#ifndef GPRS_H_
#define GPRS_H_

#ifdef GPRS_PRIVATE
#define GPRS_PUBLIC
#else
#define GPRS_PUBLIC extern
#endif

#define GPRS_DCD_PORT       GPIOD
#define GPRS_DCD_PIN        GPIO_Pin_4

//Ӧ�����ݵ�Ԫ-���ͱ�־����
#define SERVER_COMMAND_DEVICE_STATUS    65 //�ϴ��豸״̬
#define SERVER_COMMAND_SIM_ICCID        66 //��ѯ�����ϴ�SIM��CCID
#define SERVER_COMMAND_DEVICE_CONFIG    67 //������ֵ
#define SERVER_COMMAND_DEVICE_RESET     128 //��λ����
#define SERVER_COMMAND_DEVICE_MUTE      129 //��������

//Ӧ�����ݵ�Ԫ-ϵͳ���ʹ���
#define SYSTEM_TYPE    10 //�������ּ��ϵͳ

//Ӧ�����ݵ�Ԫ-�豸���ʹ���
#define DEVICE_TYPE_12_CHANNEL    8 //DDH5267������ģ��


#define GPRS_COMMAND_PREPARE         01 //��������׼��
#define GPRS_COMMAND_SINGAL         02 //SIM�����޼��ź�ǿ��
#define GPRS_COMMAND_CGATT         03 //GPRS����
#define GPRS_COMMAND_CONNECT_IP        04//ʹ��IP����TCP����
#define GPRS_COMMAND_HEARTBEAT        05//��������
#define GPRS_COMMAND_RECVPROCESS        06//���հ�����
#define GPRS_COMMAND_RECV_TIMEOUT        07//���հ���ʱ����
#define GPRS_COMMAND_CCID        8//���豸CCID


#define GPRS_COMMAND_SPECIAL         20 //�����ַ�����


#define AT_CREG         01 //����ע����Ϣ
#define AT_CGREG        02 //GPRS����ע��״̬
#define AT_CGATT        03 //���źͷ���GPRSҵ��
#define AT_CIPMUX       04 //����TCPIP����Э�鷽ʽ����·���Ƕ�·��ʽ
#define AT_CIPMODE      05 //����TCPIPӦ��ģʽ��͸�����Ƿ�͸��ģʽ
#define AT_CSTT         06 //�������������ƶ������APN���û���������
#define AT_CIICR        07 //�����ƶ�����������GPRS ��CSD��������
#define AT_CIPSHUT      8 //�ر��ƶ�����
#define AT_CIPSCONT     9 //����TCPIPӦ�ó���
#define AT_CIPSTART     10 //����TCP��UDPЭ����������
#define AT_CIPSEND      11 //��������
#define AT_CIPCLOSE     12 //�ر�TCP��UDP����
#define AT_CIFSR        13 //��ѯ����IP��ַ
#define AT_CIPSTATUS    14 //��ѯ��ǰ����״̬
#define AT_CDNSCFG      15 //������������������
#define AT_CIPSERVER    16 //����Ϊ������
#define AT_CIPCCFG      17 //����͸���������
#define AT_CLPORT       18 //���ñ��ض˿�
#define AT_CIPHEAD      19 //���ý��������Ƿ���ʾIPͷ
#define AT_CIPATS       20 //�����Զ��������ݵ�ʱ��
#define AT_CIPSPRT      21 //���÷�������ʱ�Ƿ���ʾ��>�� ��send ok 
#define AT_CIPCSGP      22 //����ΪCSD��GPRS����ģʽ
#define AT_CIPSRIP      23 //���ý�������ʱ�Ƿ���ʾ���ͷ���IP��ַ�Ͷ˿ں�
#define AT_CIPDPDP      24 //�����Ƿ���GPRS����״̬
#define AT_CIPSHOWTP    25 //�����Ƿ��������ʱ��IPͷ��ʾ����Э�� 
#define AT_RST    26 //�����Ƿ��������ʱ��IPͷ��ʾ����Э�� 
#define AT_CPIN         27 //SIM��״̬
#define AT_ATE         28 //���Կ���(ATE0-�رջ��ԣ�ATE1-��������)
#define AT_CSQ         29 //�����ź�ǿ��
#define AT_CGCLASS         30 //�ƶ�̨���
#define AT_CGDCONT        31 //���ӷ�ʽ
#define AT_IPR        32 //������ͬ��

#define AT_CCID      33//��SIM������ 


#define BUF_SIZE 257



GPRS_PUBLIC void Gprs_Init(void);
GPRS_PUBLIC void ConnectToServer(void);

typedef struct tagStatus
{
    uint8_t AT_CPIN_Flag: 1; //SIM��״̬
    uint8_t AT_CGATT_Flag: 1; //GPRS����״̬
    uint8_t GPRS_AbleToUse: 1; //GPRSģ���Ƿ��������ͨѶ
    uint8_t UartContectState: 1; //��������״̬
    uint8_t AT_CSTT_Flag: 1; //APN����
    uint8_t GRST_Flag: 1; //GPRSģ�鸴λ
    uint8_t ServerContectState: 2;//����������״̬

} Status_t, *pStatus_t;

typedef union
{
    Status_t Status_bit;
    uint8_t Status_u8;

} STATUS_BUFF;

//���ݰ��ϴ�״̬����
#define DATAPACKAGE_STARTING        0x0000  //GPRSģ����������
#define DATAPACKAGE_WAIT_CONNECT    0x1000  //�ȴ����������ӳɹ�
#define DATAPACKAGE_READY_TO_SEND   0x2000  //�����ϴ�����
#define DATAPACKAGE_WAIT_ACK        0x3000  //�ȴ�������Ӧ��
#define DATAPACKAGE_UPLOAD_DONE     0x4000  //�����ϴ����



typedef struct tagGprsInfo
{
    STATUS_BUFF Status;
    uint8_t AT_CSQ_Data;//�ź�ǿ��
    uint8_t Sim_Iccid[10];//�豸ICCID  
    uint16_t HeartBeat;//������ʱ
    uint16_t CommandCount;//ҵ����ˮ�ţ�����λ�������ݴ���״̬

} GprsInfo_t, *pGprsInfo_t;
GPRS_PUBLIC GprsInfo_t GprsInfo;

#define DATA_PACKAGE_EMPTY      0x00//���ݰ�Ϊ��
#define DATA_PACKAGE_RECEVING   0x01//���ݰ����ڽ���
#define DATA_PACKAGE_READY      0x02//���ݰ��������


typedef struct
{
    uint8_t DataPackage_Start;          //1�ֽ�������0x7E
    uint8_t DataPackage_MsgID_L;         //2�ֽ�ҵ����ˮ��(���ֽ���ǰ)
    uint8_t DataPackage_MsgID_H;
    
    uint8_t DataPackage_Version_L;       //2�ֽ�Э��汾��(���ֽ���ǰ)
    uint8_t DataPackage_Version_H;

    uint8_t DataPackage_MsgFrom_Low_L;   ////6�ֽ�Դ��ַ(���ֽ���ǰ)
    uint8_t DataPackage_MsgFrom_Low_H;
    uint8_t DataPackage_MsgFrom_Mid_L;   //
    uint8_t DataPackage_MsgFrom_Mid_H;   //    
    uint8_t DataPackage_MsgFrom_High_L;  //
    uint8_t DataPackage_MsgFrom_High_H;  //

    uint8_t DataPackage_MsgTo_Low_L;     ////6�ֽ�Ŀ�ĵ�ַ(���ֽ���ǰ)
    uint8_t DataPackage_MsgTo_Low_H;
    uint8_t DataPackage_MsgTo_Mid_L;     //
    uint8_t DataPackage_MsgTo_Mid_H;     //
    uint8_t DataPackage_MsgTo_High_L;    //
    uint8_t DataPackage_MsgTo_High_H;    //

    uint8_t DataPackage_DataLength_L;    //2�ֽ�Ӧ�õ�Ԫ���ݳ���
    uint8_t DataPackage_DataLength_H;
    
    uint8_t  DataPackage_Command;       //1�ֽ������ֽ�

    uint8_t DataPackage_Data[233];      //233�ֽ�Ӧ�õ�Ԫ����

    uint8_t DataPackage_CRCL;           //1�ֽ�CRCУ����ֽ�
    uint8_t DataPackage_CRCH;           //1�ֽ�CRCУ����ֽ�
    uint8_t DataPackage_Stop;           //1�ֽڽ�����0x7E
    uint8_t DataPackage_State;//���ݰ�״̬
} DATA_FORMAT;

typedef union _Buffer
{
    DATA_FORMAT DataBuff;
    unsigned char DataBufferChar[BUF_SIZE];
} USART_DATA_BUFFER;

GPRS_PUBLIC USART_DATA_BUFFER GprsRxBuf[2];//���ջ�����
GPRS_PUBLIC USART_DATA_BUFFER GprsTxBuf;


GPRS_PUBLIC void QueueGprsPut(uint8_t command);
GPRS_PUBLIC void HeartBeat_Timer(void);

#endif

