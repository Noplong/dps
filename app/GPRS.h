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

GPRS_PUBLIC uint8_t GprsRxBuf[512];


GPRS_PUBLIC void Gprs_Init(void);  
GPRS_PUBLIC void ConnectToServer(void);

#endif

