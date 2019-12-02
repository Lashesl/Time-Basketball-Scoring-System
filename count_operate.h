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

 	 ASCORE0 EQU 20H
	  ASCORE1 EQU 21H
	  BSCORE0 EQU 22H
	  BSCORE1 EQU 23H

MOV ASCORE0,#00H	  ;设定初值
	  MOV ASCORE1,#00H
	  MOV BSCORE0,#00H
	  MOV BSCORE1,#00H


FSCZ: 	MOV TH0,#3CH    	;定时器预置初值  
	   	MOV TL0,#0B0H
     	MOV R2,#0
	   	MOV R3,#0
	   	MOV R4,#0
	   	MOV R5,#0      	;倒计时器初始化时间为01.00
	   	CLR 00H		  		/*位地址00H用来进行倒计时器暂停（清零）和
继续（置1）的功能切换*/
	   	CLR 01H
	   	CLR 02H
	CLR 04H       		/*当倒计时器的分数十位从0减1变9时，04H
置1 来控制倒计时器的结束*/

ST0:  	CJNE ASCORE1,#10,LP1	/*对分数的十位进行判断，超过9则表示计时已超
100分钟范围，停止计时*/  
   			CJNE BSCORE1,#10,LP1
     	AJMP LP2
LP1:  	JC START	      	;<10
LP2:  	CLR 00H	      		;≥10
	  	CLR TR0
     	MOV ASCORE1,#0       
	   	MOV ASCORE0,#0
	   	MOV BSCORE1,#0
	   	MOV BSCORE0,#0
SHOW1:LCALL SSMGSHOW  	;数码管显示比值0000
	   	LCALL KEY
	   	CJNE A,#6,SHOW1	;不断扫描键盘直至复位键按下
	   	AJMP FSCZ

START:	LCALL SMGSHOW   
	   	LCALL KEY

LOOP: 	LCALL KEY   	;键盘扫描	  
	   	CJNE A,#0F0H,AUP 	;有键按下时跳转
	   	AJMP LOOP
AUP:   	CJNE A,#10,BUP  
      	LCALL AAUP       	;A队+1分子程序
      	AJMP LOOP
BUP: CJNE A,#11, SSHOW     
	   	LCALL TDOWN     ;B队+1分子程序
	   	AJMP LOOP
SSHOW: CJNE A,#6,SCORECH 
      	LCALL SSMGSHOW       	;分数显示子程序
      	AJMP LOOP
SCORECH: CJNE A,#2, RES           ;分数对换子程序
			LCALL SCOREC
			AJMP LOOP  
RES:  	CJNE A,#7,STA1
     	AJMP FSCZ

;A队+1分子程序
;---------------------
AAUP: 	NOP
UAS0: 	INC ASCORE0
		CJNE ASCORE0,#10,OUT4  	;ASCORE0满10清0向ASCORE1进1
UAS1:	MOV ASCORE0,#0
		INC ASCORE1
OUT4: 	RET

  
;B队+1分子程序
;---------------------
BBUP: 	NOP
UBS0: 	INC BSCORE0
		CJNE BSCORE0,#10,OUT5  	;ASCORE0满10清0向ASCORE1进1
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
OUT5: 	RET


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
OUT6:	RETI

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
	    MOV A,B		      		;显示右边分数十位
		SETB SMG2				;选中数码管2
		CALL SMG
		MOV R6,#4 
		CALL DELAY				;延时2ms
		CLR SMG2		;关闭数码管2

	   	MOV A,BSCORE0
		MOV B,#10
		DIV AB
	    MOV A,B	      	;显示左边分数个位
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
	    MOV A,B	      	;显示左边分数十位
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
