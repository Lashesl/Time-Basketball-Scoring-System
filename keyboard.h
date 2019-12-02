ORG 0000H
AJMP MAIN				  		;主程序入口
ORG 0030H

MAIN: 	MOV SP,#60H		   	;设堆栈
KEY:  	MOV P2,#0F0H			;查有键按下
	  	MOV A,#0F0H
	  	CJNE A,P2,K01
	  	AJMP KEY
K01:  	MOV R6,#20
      	ACALL DELAY	     	;延时20ms去抖
	  	MOV P2,#0F0H     		;重查有键按下
	  	CJNE A,P2,K02
	  	AJMP KEY

K02:  	MOV P2,#0FEH	   		;行线1变低
	  	MOV A,P2
	  	CJNE A,#0FEH,YES    	;查何键按下
      	MOV P2,#0FDH	   		;行线2变低
      	MOV A,P2
      	CJNE A,#0FDH,YES   	;查何键按下
	  	MOV P2,#0FBH	   		;行线3变低
	  	MOV A,P2
 	  	CJNE A,#0FBH,YES    	;查何键按下
	  	MOV P2,#0F7H	   		;行线4变低 
	  	MOV A,P2
	  	CJNE A,#0F7H,YES   	;查何键按下
	  	AJMP K02
YES:  	ACALL KEY_VALUES   	;取键值存入A
      	LCALL KEYRE
      	AJMP KEY

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
END
