ORG 0000H
AJMP MAIN					;主程序入口
ORG 000BH          
AJMP TIME0         		;定时器0中断入口
ORG 0030H

MAIN: NOP					;用于程序指令的对齐
	  MOV SP,#60H			;设堆栈
    MOV P0,#00H
	  SETB EA          	;允许全局中断
	  SETB ET0          	;允许定时器中断  
	  MOV TMOD,#01H   	;定时模式1
	  MOV R1,#0		  	;通过R1来记录定时器溢出中断个数
	  CLR 01H			 	/*数码管显示时，位地址01H用来区别字母（置
1）与分秒数（清零）*/
	  SCK BIT P0.4
	  RCK BIT P0.5
	  RST BIT P0.6
	  DAT BIT P0.7
	  SMG1 BIT P0.3
	  SMG2 BIT P0.2
	  SMG3 BIT P0.1
	  SMG4 BIT P0.0
	  BEEP BIT P1.3

SECOND0 EQU 24H
	  SECOND1 EQU 25H
	  MINUTE0 EQU 26H
	  MINUTE1 EQU 27H

  MOV SECOND0,#00H
	  MOV SECOND1,#00H
	  MOV MINUTE0,#00H
	  MOV MINUTE1,#00H


DJSQ: 	MOV TH0,#86H    	;定时器预置初值	
		MOV TL0,#0A0H
     	MOV MINUTE1,#1
		MOV MINUTE0,#2
		MOV SECOND1,#0
		MOV SECOND0,#0      	;倒计时器初始化时间为12.00

		CLR 00H				/*位地址00H用来进行倒计时器暂停（清零）和继续（置1）的功能切换*/
		CLR 01H
		CLR 02H			
		CLR 04H       		/*当倒计时器的分数十位从0减1变9时，04H置1来控制倒计时器的结束*/

LOOP:	LCALL SSMGSHOW   	;数码管显示和键盘扫描
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
		CJNE A,#0F0H,STOP1 	 ;有按键按下则跳转
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


CLOSE:	CJNE A,#14,REST 	;按下15关闭声音键
		SETB BEEP				;蜂鸣器消声
		AJMP OK
REST:	CJNE A,#12,EN	   ;时间复位操作12
		AJMP DJSQ        	;返回到倒计时器子程序开始处	 即复位时间
EN:	CJNE A,#15,OK
		RET					 ;结束比赛选项

;时间上升子程序
;---------------------
TUP: 	NOP
US0: 	INC SECOND0
		CJNE SECOND0,#10,OUT2  	;SECOND0满10清0向SECOND1进1
US1:	MOV SECOND0,#0
		INC SECOND1
		CJNE SECOND1,#6,OUT2   	;SECOND1满6清0向MINUTE0进1
UM0:	MOV SECOND1,#0
		INC MINUTE0
		CJNE MINUTE0,#10,OUT2  	;MINUTE0满10清0向MINUTE1进1
UM1: 	MOV MINUTE0,#0
		INC MINUTE1
OUT2: 	RET


;时间下降子程序
;-------------------------
TDOWN:NOP
DR50: 	CJNE SECOND0,#0,DR51	/*R5为0时减1变9，并向R4借位（跳转），否则减1后退出子程序*/
		MOV SECOND0,#9
		AJMP DR40
DR51: 	DEC SECOND0
		AJMP OUT3
DR40: 	CJNE SECOND1,#0,DR41   	/*R4为0时减1变5，并向R3借位（跳转），否则减1后退出子程序*/
		MOV SECOND1,#5
		AJMP DR30
DR41: 	DEC SECOND1
		AJMP OUT3
DR30: 	CJNE MINUTE0,#0,DR31   	/*R3为0时减1变9，并向R2借位（跳转），否则减1后退出子程序*/
			MOV MINUTE0,#9
		AJMP DR20
DR31:	 DEC MINUTE0
		AJMP OUT3
DR20: 	CJNE MINUTE1,#0,DR21   	;R2为0时减1变9,，否则减1后退出子程序
		MOV MINUTE1,#9
		SETB 04H          ;04H置1以便进行倒计时结束判断
		AJMP OUT3
DR21: 	DEC MINUTE1
OUT3: 	RET

;定时器0中断服务子程序
;------------------------
TIME0:	INC R1           	; R1记录定时器溢出中断次数
	 	MOV TH0,#3CH
	  	MOV TL0,#0B0H    ;重装定时器预置初值
	  	CJNE R1,#10,OUT4  ;R1满10清0，时间下降
	  	MOV R1,#0
DJ:  	LCALL TDOWN
OUT4: 	RETI

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
OUT6:	RETI

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
	    MOV A,B		      		;显示秒数十位
		SETB SMG2				;选中数码管2
		CALL SMG
		MOV R6,#4 
		CALL DELAY				;延时2ms
		CLR SMG2		;关闭数码管2

	   	MOV A,MINUTE0
		MOV B,#10
		DIV AB
	    MOV A,B	      	;显示分数个位
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
	    MOV A,B	      	;显示分数十位
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
		MOV A,#11				;键号11	 （B队分数+1）
		AJMP OUT1
K12:  	CJNE A,#0E7H,K13
		MOV A,#12				;键号12	  （时间复位键）
		AJMP OUT1
K13:  	CJNE A,#0EBH,K14
		MOV A,#13				;键号13	   （选择倒计时开始键）
		AJMP OUT1
K14:  	CJNE A,#0EDH,K15
		MOV A,#14				;键号14		（关闭声音）
		AJMP OUT1	
K15:  	CJNE A,#0EEH,OUT1
		MOV A,#15				;键号15		 （结束比赛）
OUT1: 	RET

;数码管段码表
;-------------------
SMGTAB: DW 0FC60H,0DAF2H	;对应数字0、1、2、3
		DW 66B6H,0BEE0H		;对应数字4、5、6、7
		DW 0FEF6H,8E7CH		;对应数字8、9，字母F、U
		DW 0EC9CH,0FCAEH		;对应字母N、C、O、K
END
