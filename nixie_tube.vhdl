ORG 0000H
AJMP MAIN					;主程序入口
ORG 0030H

MAIN: 	NOP				;用于程序指令的对齐
	  	MOV SP,#60H		;设堆栈
CLR 01H			   /*数码管显示时，位地址01H用来区别字母（置
1）与分秒数*/
	  	SCK BIT P0.4
	  	RCK BIT P0.5
	  	RST BIT P0.6
	  	DAT BIT P0.7
	  	SMG1 BIT P0.3
	  	SMG2 BIT P0.2
	  	SMG3 BIT P0.1
	  	SMG4 BIT P0.0
	  	BEEP BIT P1.3
SMGSHOW:NOP
     	MOV A,R5		;显示秒数个位
	  	SETB SMG1   ;选中数码管1
	  	CALL SMG	   	;调用1位数码管显示子程序
	  	MOV R6,#4
	  	CALL DELAY  ;延时2ms
	  	CLR SMG1		;关闭数码管1
	  
	  	MOV A,R4     ;显示秒数十位
	  	SETB SMG2	;选中数码管2
	  	CALL SMG
	  	MOV R6,#4 
	  	CALL DELAY	;延时2ms
	  	CLR SMG2	   	;关闭数码管2

	  	MOV A,R3     	;显示分数个位
	  	SETB SMG3	   	;选中数码管3
     	JB 01H,SM02	   ;01H为1（显示的是字母）跳转
SM01: 	SETB 02H	    	;01H为0时（显示的是时间），02H置1
SM02: 	CALL SMG
     	MOV R6,#4
	  	CALL DELAY	  	;延时2ms
	  	CLR SMG3		   ;关闭数码管3

	  	MOV A,R2     	;显示分数十位
	  	SETB SMG4	  	;选中数码管4
	  	CALL SMG
	  	MOV R6,#4 
	  	CALL DELAY	  	;延时2ms
	  	CLR SMG4		   ;关闭数码管4
	  	RET

;1位数码管显示子程序（向74HC595发送1个字节）
;--------------------------------------------
SMG: 	MOV DPTR,#SMGTAB ;装入表头
	  	MOVC A,@A+DPTR  ;查表取值
	   	JBC 02H,DQ0	    /*02H为1时表示时间显示中的数码管3的显示，
A的值应加1显示小数点*/
	   	AJMP S0
DQ0:  	ADD A,#1	
S0:	   	CLR RST  ;RST为输出使能控制，低电平时使输出使能，启动74HC595
	   	CLR RCK    		;RCK先清0
	   	MOV R0,#8 			;进行8位数据的传输
S1:    CLR SCK  			;SCK先清0
	   	RRC A    ;将A中8位数据分别从低到高取出，放到进位位CY中
	  	JC S2      			;当CY=1时将DAT置1,当CY=0时，将DAT清0
	  	CLR DAT
	  	AJMP S3
S2:   	SETB DAT
S3:   	SETB SCK  ;SCK置1产生1个上升沿，使移位寄存器接受新的bit
	  	DJNZ R0,S1 			;判断8位数据是否传输完毕
	  	SETB RCK    		/*移位结束后RCK置1,产生1个上升沿更新存储
寄存器中的显示数据*/
	  	RRC A
	  	RET

;延时子程序
;-----------------
DELAY:	NOP  
DLY0: 	MOV R7,#250
DLY1: 	DJNZ R7,DLY1
	  	DJNZ R6,DLY0
	  	RET

;数码管段码表
;-------------------
SMGTAB:DW 0FC60H,0DAF2H	;对应数字0、1、2、3
	   	DW 66B6H,0BEE0H		;对应数字4、5、6、7
	   	DW 0FEF6H,8E7CH		;对应数字8、9，字母F、U
	   	DW 0EC9CH,0FCAEH		;对应字母N、C、O、K
END
