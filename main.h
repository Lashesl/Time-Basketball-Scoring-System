ORG 0000H		
AJMP MAIN		 ;主程序入口
ORG 0030H
AJMP JINT0 		  ;外部中断0入口
ORG 000BH
AJMP TIME0       ;定时器0中断入口
ORG 0030H

MAIN: NOP
	  MOV SP,#60H    ;设置堆栈
	  MOV P0, #00H
	  SETB EA		  ;允许全局中断
	  SETB ET0		  ;允许定时器中断
	  SETB EX0		  ;允许外部中断0中断
	  SETB PX0		  ;设外部中断0为高优先级
	  CLR IT0		  ;设外部中断0低电平触发
	  MOV TMOD,#01H	  ;定时模式1
	  MOV R1,#0		  ;通过R1来记录定时器中断个数
	  CLR 01H		  /*数码管显示时，位地址01H用来区别字母（置1）与分秒数（清0）*/
	
	  ASCORE0 EQU 20H
	  ASCORE1 EQU 21H
	  BSCORE0 EQU 22H
	  BSCORE1 EQU 23H
	  SECOND0 EQU 24H
	  SECOND1 EQU 25H
	  MINUTE0 EQU 26H
	  MINUTE1 EQU 27H

	  SCK BIT P0.4
	  RCK BIT P0.5
	  RST BIT P0.6
	  DAT BIT P0.7
	  SMG1 BIT P0.3
	  SMG2 BIT P0.2
	  SMG3 BIT P0.1
	  SMG4 BIT P0.0
	  BEEP BIT P1.3

	  MOV ASCORE0,#00H	  ;设定初值
	  MOV ASCORE1,#00H
	  MOV BSCORE0,#00H
	  MOV BSCORE1,#00H
	  MOV SECOND0,#00H
	  MOV SECOND1,#00H
	  MOV MINUTE0,#00H
	  MOV MINUTE1,#00H

MAD0:LCALL KEY        ;调用键盘扫描子程序
     CJNE A,#0F0H,MBJS;判断是否有键按下
	 AJMP MAD0		  ;无键按下，循环

MBJS:CJNE A,#0,DJS
     LCALL MBJSQ	  ;0键按下，进入秒表计时器子程序
DJS: CJNE A,#1,OTHER
     LCALL DJSQ		  ;1键按下，进入倒计时器子程序
OTHER:MOV MINUTE1,#10	   /*其他键按下，数码管显示func，*/
      MOV MINUTE0,#11
	  MOV SECOND1,#12
	  MOV SECOND0,#13
	  SETB 01H

SHOW:LCALL SSMGSHOW	   /*调用数码管显示子程序，不断显示func，直至有键按下*/

MAD1:LCALL KEY
     CJNE A,#0F0H,MBJS	;有键按下则跳转
	 AJMP SHOW


;秒表计时器子程序
;-----------------	

MBJSQ:MOV MINUTE1,#0		   ;组成分数
      MOV MINUTE0,#0
	  MOV SECOND1,#0        ;组成秒数，初始值00：00
	  MOV SECOND0,#0
	  CLR 00H		   /*位地址00H用来进行秒表计时器启动（清0）和停止（置1）的功能切换*/
	  CLR 01H		   /*数码管显示时，01H用来区别字母（置1）与分秒数（清0）*/
	  CLR 02H		   /*当数码管显示时间时，02H用来针对SMG3显示其小数点位（置1）*/
	  CLR 03H
		MOV R2,#0
		MOV R3,#0
		MOV R4,#0
		MOV R5,#0

ST0:  MOV R2,MINUTE1
	  CJNE R2,#10,LP1  /*对分数的十位进行判断，超过9则表示计时已超过100分钟范围，停止计时*/

	  AJMP LP2       ;超出范围
LP1:  JC START		 ;MINUTE1<10
LP2:  CLR 00H	     ;MINUT1>=10
	  CLR TR0
	  MOV MINUTE1,#9
	  MOV MINUTE0,#9
	  MOV SECOND1,#5
	  MOV SECOND0,#9
SHOW1:LCALL TSMGSHOW	   ;数码管显示时间99.59
	  LCALL KEY
	  CJNE A,#12,SHOW1	;不断扫描键盘直至复位12键按下
	  AJMP RES0  		;key扫描是否对a有更改，res0为复位

START:LCALL TSMGSHOW
      LCALL KEY
	  CJNE A,#0F0H,SCOREC0    /*有键按下跳转，无键按下继续超时判断、显示和键盘扫描*/
	  AJMP ST0

SCOREC0:CJNE A,#10,AAUP1		 ;A队+1
	 	LCALL CHAN1
	   LCALL AAUP
		LCALL CHAN2
	   AJMP ST0
AAUP1: CJNE A,#11,BBUP1		 ;B队+1
	  LCALL CHAN1
	   LCALL BBUP
	LCALL CHAN2
	   AJMP ST0
BBUP1: CJNE	A,#6,GO         ;显示分数
	  LCALL	CHAN1
	   LCALL SSMGSHOW
	LCALL CHAN2
	   AJMP ST0

GO:   CJNE A,#4,RES0	 ;4键（启动/停止）按下是否重新计时
      JBC 00H,STOP0		  ;00H为0时为启动，为1时为停止
	  MOV TH0,#86H		  ;为启动功能时，暂停之后不能继续计时，装定时预置值
	  MOV TL0,#0A0H
	  SETB TR0			  ;启动定时器
	  SETB 00H
	  AJMP ST0
STOP0:LCALL TSMGSHOW
	  CLR TR0			  ;为停止功能，停止定时器，停止计时
      AJMP ST0
RES0:LCALL TSMGSHOW
	 JB 00H,ST0			  ;停止计时时才能复位
     CJNE A,#12,ST0		   ;12为复位键
	 AJMP MBJSQ			   ;按下复位键回到子程序开始
	 RET


;倒计时器子程序
;------------------
DJSQ: 	MOV TH0,#86H    	;定时器预置初值	
		MOV TL0,#0A0H
		MOV MINUTE1,#1
		MOV MINUTE0,#2
		MOV SECOND1,#0
		MOV SECOND0,#0      	;倒计时器初始化时间为12.00
		MOV R2,#1
		MOV R3,#2
		MOV R4,#0
		MOV R5,#0      	;倒计时器初始化时间为12.00

		CLR 00H				/*位地址00H用来进行倒计时器暂停（清零）和继续（置1）的功能切换*/
		CLR 01H
		CLR 02H
		SETB 03H			
		CLR 04H       		/*当倒计时器的分数十位从0减1变9时，04H置1来控制倒计时器的结束*/

LOOP:	LCALL TSMGSHOW   	;数码管显示和键盘扫描
		LCALL KEY	
		CJNE A,#0F0H,UP 	;有键按下时跳转
		AJMP LOOP
UP:   	CJNE A,#8,DOWN   	;8键为+时间
		LCALL TUP       	;up键按下时调用时间上升子程序
		AJMP LOOP
DOWN:   CJNE A,#9,RES		;9键为-时间
		LCALL TDOWN     	;down键按下时调用时间下降子程序
		AJMP LOOP
RES:	CJNE A,#12,STA1      ;复位键按下回到初始状态
		AJMP DJSQ
STA1: 	CJNE A,#13,LOOP      ;13键为开始
		SETB TR0        	;开始键按下时启动定时器
		CLR 04H        	    ;排除up和down时分数十位从0减1变9的影响

L1:   	JNB 04H,L2      	;判断倒计时是否结束
		CLR TR0            	 /*当04H置1时分数十位从0减1变9，即倒计时时间已为00.00,倒计时结束*/
		CLR BEEP        	;蜂鸣器发声
		SETB 01H		
		MOV MINUTE1,#14   
		MOV MINUTE0,#15
		MOV SECOND1,#14
		MOV SECOND0,#15
OK:		ACALL TSMGSHOW   	;数码管显示OKOK
		ACALL KEY
		CJNE A,#0F0H,CLOSE ;不断显示OKOK和键盘扫描，直至关闭键按下
		AJMP OK

L2:   	ACALL TSMGSHOW  		 ;数码管显示和键盘扫描
KEY0: 	ACALL KEY
		CJNE A,#0F0H,AUP 	 ;有按键按下则跳转
		AJMP L1
AUP:	CJNE A,#10,BUP 		;10键为A加分1
		LCALL CHAN1
		LCALL AAUP			 ;10键按下时调用A加1分子程序
		LCALL CHAN2
		AJMP L1
BUP:   	CJNE A,#11,FSSHOW   	;11键为B+1分
		LCALL CHAN1
		LCALL BBUP       	;11键按下时调用B加1分子程序
		LCALL CHAN2
		AJMP L1
FSSHOW: CJNE A,#6,STOP1	 ;根据6键显示分数
        LCALL CHAN1
		LCALL SSMGSHOW
		LCALL CHAN2
		AJMP L1
STOP1:	CJNE A,#5,RES1		;按下暂停/继续键  5键
		JBC 00H,CONT     	;00H为0时暂停，00H为1时继续
		SETB 00H         	;00H置1以便下次该键按下时实现继续功能
		CLR TR0				;停止定时器
		AJMP L1
CONT: 	MOV TH0,#86H     	;重装定时预置值
		MOV TL0,#0A0H
		SETB TR0         	;启动定时器
		AJMP L1
RES1: 	CJNE A,#12,L1    	;按下时间复位键
		CLR TR0          		;停止计时
		AJMP DJSQ        	;返回到倒计时器子程序开始处


CLOSE:	CJNE A,#14,SCORE1		;按下15关闭声音键
		SETB BEEP				;蜂鸣器消声
		AJMP OK
SCORE1:CJNE A,#2,SCORESH		;交换分数
		LCALL SCOREC
		AJMP OK
SCORESH:CJNE A,#6,REST
		LCALL SSMGSHOW	;显示分数 6
		AJMP OK
REST:	CJNE A,#12,EN	   ;时间复位操作12
		AJMP DJSQ        	;返回到倒计时器子程序开始处	即复位时间
EN:		CJNE A,#15,OK
		RET					 ;结束比赛选项


;时间上升子程序
;---------------------
TUP: 	NOP
US0: 	INC R5
		CJNE R5,#10,OUT2  	;SECOND0满10清0向SECOND1进1
US1:	MOV R5,#0
		INC R4
		CJNE R4,#6,OUT2   	;SECOND1满6清0向MINUTE0进1
UM0:	MOV R4,#0
		INC R3
		CJNE R3,#10,OUT2  	;MINUTE0满10清0向MINUTE1进1
UM1: 	MOV R3,#0
		INC R2
OUT2: 	RET

;交换1
;-----------------
CHAN1:MOV SECOND0,R5
		MOV SECOND1,R4
		MOV MINUTE0,R3
		MOV MINUTE1,R2
		RET

;交换2
;--------------------
CHAN2: 	MOV R5,SECOND0
		MOV R4,SECOND1
		MOV R3,MINUTE0
		MOV R2,MINUTE1
		RET


;时间下降子程序
;-------------------------
TDOWN:NOP
DR50: 	CJNE R5,#0,DR51	/*R5为0时减1变9，并向R4借位（跳转），否则减1后退出子程序*/
		MOV R5,#9
		AJMP DR40
DR51: 	DEC R5
		AJMP OUT3
DR40: 	CJNE R4,#0,DR41   	/*R4为0时减1变5，并向R3借位（跳转），否则减1后退出子程序*/
		MOV R4,#5
		AJMP DR30
DR41: 	DEC R4
		AJMP OUT3
DR30: 	CJNE R3,#0,DR31   	/*R3为0时减1变9，并向R2借位（跳转），否则减1后退出子程序*/
		MOV R3,#9
		AJMP DR20
DR31:	 DEC R3
		AJMP OUT3
DR20: 	CJNE R2,#0,DR21   	;R2为0时减1变9,，否则减1后退出子程序
		MOV MINUTE1,#9
		SETB 04H          ;04H置1以便进行倒计时结束判断
		AJMP OUT3
DR21: 	DEC R2
OUT3: 	RET


;A队+1分子程序
;---------------------
AAUP: 	NOP
UAS0: 	INC ASCORE0
		MOV R5,ASCORE0
		CJNE R5,#10,OUT4  	;ASCORE0满10清0向ASCORE1进1
UAS1:	MOV ASCORE0,#0
		INC ASCORE1
OUT4: 	RET


;B队+1分子程序
;---------------------
BBUP: 	NOP
UBS0: 	INC BSCORE0
		MOV R3,BSCORE0
		CJNE R3,#10,OUT5  	;ASCORE0满10清0向ASCORE1进1
UBS1:	MOV BSCORE0,#0
		INC BSCORE1
OUT5: 	RET

;AB分数交换子程序
;---------------------
SCOREC: NOP
		MOV A,ASCORE0
		XCH A,BSCORE0
		MOV ASCORE0,A
		MOV A,ASCORE1
		XCH A,BSCORE1
		MOV BSCORE1,A
OUT6: 	RET


;外部中断0中断服务子程序（系统复位）
;---------------------------------------
JINT0:	CLR TR0
		MOV DPTR,#MAIN
		POP ACC				;弹出原返回地址	
		POP ACC
		PUSH DPH				;将主程序入口地址入栈
		PUSH DPL
		RETI

;定时器0中断服务子程序
;------------------------
TIME0:	INC R1		;R1记录定时器溢出中断次数
		MOV TH0,#86H
		MOV TL0,#0A0H		;重装定时器预置初值
		CJNE R1,#10,OUT6		;R1满10清0
		MOV R1,#0
SELET:	JB 03H,DJ
JS:		LCALL TUP		;当前为秒表计时器时，时间上升
		AJMP OUT6
DJ:		LCALL TDOWN	;当前为倒计时器时，时间下降
OUT7:	RETI

;四位数码管动态显示时间子程序
;---------------------------
TSMGSHOW:NOP
		MOV A,SECOND0
		MOV B,#10
		DIV AB
	    MOV A,B				;显示秒数个位
		SETB SMG1     		;选中数码管1
		CALL SMG			;调用1位数码管显示子程序
		MOV R6,#4
		CALL DELAY    			;延时2ms
		CLR SMG1				;关闭数码管1
	
		MOV A,SECOND1
		MOV B,#10
		DIV AB
	    MOV A,B				;显示秒数十位
		SETB SMG2				;选中数码管2
		CALL SMG
		MOV R6,#4 
		CALL DELAY				;延时2ms
		CLR SMG2		;关闭数码管2

		MOV A,MINUTE0
		MOV B,#10
		DIV AB
	    MOV A,B		;显示分数个位
		SETB SMG3			;选中数码管3
		JB 01H,SM02		;01H为1（显示的是字母）跳转
SM01: 	SETB 02H			;01H为0时（显示的是时间），02H置1
SM02: 	CALL SMG
		MOV R6,#4
		CALL DELAY			;延时2ms
		CLR SMG3			;关闭数码管3

		MOV A,MINUTE1
		MOV B,#10
		DIV AB
	    MOV A,B		;显示分数十位
		SETB SMG4			;选中数码管4
		CALL SMG
		MOV R6,#4 
		CALL DELAY		;延时2ms
		CLR SMG4			;关闭数码管4
		RET


;四位数码管动态显示分数子程序
;---------------------------
SSMGSHOW:NOP
		MOV A,ASCORE0
		MOV B,#10
		DIV AB
	    MOV A,B				;显示右边分数个位
		SETB SMG1     		;选中数码管1
		CALL SMG			;调用1位数码管显示子程序
		MOV R6,#4
		CALL DELAY    			;延时2ms
		CLR SMG1				;关闭数码管1
	
		MOV A,ASCORE1
		MOV B,#10
		DIV AB
	    MOV A,B				;显示右边分数十位
		SETB SMG2				;选中数码管2
		CALL SMG
		MOV R6,#4 
		CALL DELAY				;延时2ms
		CLR SMG2		;关闭数码管2

		MOV A,BSCORE0
		MOV B,#10
		DIV AB
	    MOV A,B		;显示左边分数个位
		SETB SMG3			;选中数码管3
		JB 01H,SSM02		;01H为1（显示的是字母）跳转
SSM01: 	SETB 02H			;01H为0时（显示的是时间），02H置1
SSM02: 	CALL SMG
		MOV R6,#4
		CALL DELAY			;延时2ms
		CLR SMG3			;关闭数码管3

		MOV A,BSCORE1
		MOV B,#10
		DIV AB
	    MOV A,B		;显示左边分数十位
		SETB SMG4			;选中数码管4
		CALL SMG
		MOV R6,#4 
		CALL DELAY		;延时2ms
		CLR SMG4			;关闭数码管4
		RET

;1位数码管显示子程序（向74HC595发送1个字节）
;--------------------------------------------
SMG:  	MOV DPTR,#SMGTAB	;装入表头
		MOVC A,@A+DPTR  ;查表取值
		JBC 02H,DQ0		/*02H为1时表示时间显示中的数码管3的显示，A的值应加1显示小数点*/
		AJMP S0
DQ0: 	ADD A,#1	
S0:		CLR RST         	/*RST为输出使能控制，低电平时使输出使能，启动74HC595*/
		CLR RCK         	;RCK先清0
		MOV R0,#8       	;进行8位数据的传输
S1:   	CLR SCK  		;SCK先清0
		RRC A      		/*将A中8位数据分别从低到高取出，放到进位位CY中*/
		JC S2           	;当CY=1时将DAT置1,当CY=0时，将DAT清0
		CLR DAT
		AJMP S3
S2:   	SETB DAT
S3:   	SETB SCK   			/*SCK置1产生1个上升沿，使移位寄存器接受新的bit*/
		DJNZ R0,S1     	;判断8位数据是否传输完毕
		SETB RCK        	/*移位结束后RCK置1,产生1个上升沿更新存储寄存器中的显示数据*/
		RRC A
		RET

;键盘扫描子程序
;-----------------
KEY:  	MOV P2,#0F0H		;查有键按下
		MOV A,#0F0H
		CJNE A,P2,K01
		AJMP OUT
K01:  	MOV R6,#20
		ACALL DELAY		;延时20ms去抖
		MOV P2,#0F0H     ;重查有键按下
		CJNE A,P2,K02
		AJMP OUT

K02:  	MOV P2,#0FEH		;行线1变低
		MOV A,P2
		CJNE A,#0FEH,YES	;查何键按下
		MOV P2,#0FDH		;行线2变低
		MOV A,P2
		CJNE A,#0FDH,YES	;查何键按下
		MOV P2,#0FBH		;行线3变低
		MOV A,P2
		CJNE A,#0FBH,YES ;查何键按下
		MOV P2,#0F7H		;行线4变低
		MOV A,P2
		CJNE A,#0F7H,YES		;查何键按下
		AJMP K02
YES:  	ACALL KEY_VALUES		;取键值存入A
		LCALL KEYRE
OUT:  	RET

;查键释放子程序
;-----------------------
KEYRE:	PUSH ACC		;保护ACC的值
REL0: 	MOV P2,#0F0H     		;查键释放
		MOV A,#0F0H
		CJNE A,P2,REL0
		MOV R6,#20				;输入延时10ms参数
		LCALL DELAY      		;延时10ms去抖
		CJNE A,P2,REL0   		;重查键释放
		POP ACC
		RET

;延时子程序
;-----------------
DELAY:	NOP  
DLY0: 	MOV R7,#250
DLY1:	DJNZ R7,DLY1
		DJNZ R6,DLY0
		RET

;查键值子程序
;-----------------	
KEY_VALUES:NOP
		CJNE A,#77H,K1
		MOV A,#0				;键号0（选择秒表计时器）
		AJMP OUT1
K1:   	CJNE A,#7BH,K2
		MOV A,#1				;键号1（选择倒计时器）
		AJMP OUT1
K2:   	CJNE A,#7DH,K3
		MOV A,#2				;键号2（选择分数对换）
		AJMP OUT1
K3:   	CJNE A,#7EH,K4
		MOV A,#3				;键号3
		AJMP OUT1
K4:   	CJNE A,#0B7H,K5
		MOV A,#4				;键号4（秒表计时器的启动/停止键）
		AJMP OUT1
K5:   	CJNE A,#0BBH,K6
		MOV A,#5				;键号5（倒计时器的暂停/继续键）
		AJMP OUT1
K6:   	CJNE A,#0BDH,K7
		MOV A,#6H				;键号6（选择数码管显示分数）
		AJMP OUT1
K7:   	CJNE A,#0BEH,K8
		MOV A,#7				;键号7（选择分数复位）
		AJMP OUT1
K8:   	CJNE A,#0D7H,K9
		MOV A,#8				;键号8（倒计时器的时间+1）
		AJMP OUT1
K9:   	CJNE A,#0DBH,K10
		MOV A,#9				;键号9（倒计时器的时间-1）
		AJMP OUT1
K10:  	CJNE A,#0DDH,K11
		MOV A,#10				;键号10	（A队分数+1）
		AJMP OUT1
K11:  	CJNE A,#0DEH,K12
		MOV A,#11				;键号11	（B队分数+1）
		AJMP OUT1
K12:  	CJNE A,#0E7H,K13
		MOV A,#12				;键号12	（时间复位键）
		AJMP OUT1
K13:  	CJNE A,#0EBH,K14
		MOV A,#13				;键号13	（选择倒计时开始键）
		AJMP OUT1
K14:  	CJNE A,#0EDH,K15
		MOV A,#14				;键号14		（关闭声音）
		AJMP OUT1	
K15:  	CJNE A,#0EEH,OUT1
		MOV A,#15				;键号15		（结束比赛）
OUT1: 	RET

;数码管段码表
;-------------------
SMGTAB: DW 0FC60H,0DAF2H	;对应数字0、1、2、3
		DW 66B6H,0BEE0H		;对应数字4、5、6、7
		DW 0FEF6H,8E7CH		;对应数字8、9，字母F、U
		DW 0EC9CH,0FCAEH		;对应字母N、C、O、K
END
