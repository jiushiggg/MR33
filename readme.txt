��Ҫ�ĵ��Թ��ߣ�
putty:ʹ��telnet�򴮿ڴ�ӡlog��SSH�˿ڵ�¼��
tftpd32.exe������һ��tftp������������ĸ����PC�˵��ļ����䡣


ʹ�õ�TI��
simplelink_cc2640r2_sdk_2_20_00_49

����ѡ��C99
properties->Build->Language Options->C99

�ѱ������Ӱ�bin�ļ��ŵ�����Ŀ¼�У�
/home/elinker/bin/rf.bin
/home/elinker/bin/ble.bin

linux�˳���ָ�
mv
tftp -g -r xxx IPaddress
chmod


linux��¼�˻���root�����룺hanshow-imx6


cp2tftp.bat�������ļ���
ɾ��tftpĿ¼���µ�bin�ļ������Լ�Ŀ¼�µ��ļ����Ƶ�tftpĿ¼�¡�


ʹ��GRAM
http://dev.ti.com/tirex/explore/node?node=AKS5oOWR4K-9ijBgtvOWNw__krol.2c__LATEST
Using Cache as GPRAM
In CCS; Project -> Properties -> ARM Linker -> Advanced Options -> Command File Preprocessing.  Add 
GPRAM_AS_RAM=1

In CCS; Project -> Properties -> ARM Compiler-> Predefined Symbols. Add
GPRAM_AS_RAM


Using AUX RAM as GPRAM
In CCS; Project -> Properties -> ARM Linker -> Advanced Options -> Command File Preprocessing.  Add 
AUX_AS_RAM=1

Hex��ͷ ack
f0f0f0f00100010001004110010001000000151600003aa9
f0f0f0f00100010001004110010001000000151600027889


HEX msg
F0 F0 F0 F0 01 00 01 00 02 00 41 10 01 00 01 00 00 00 15 16 00 00 13 CB

