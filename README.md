ScriptEngine
============
Studying Script language.
This language is not support `GC`...


ForExample)
```php  
/*
 * FizzBuzzを出力する
 * 3と5で割り切れる値のとき ... FizzBuzz
 * 3で割り切れる値 ... Fizz
 * 5で割り切れる値 ... Buzz
 * それ以外 ... 数字を出力
 */
function main(){
	for( $i = 1 ; $i <= 100 ; $i += 1 ){
		$s = "";
		if( $i % 3 == 0 ) $s += "Fizz";
		if( $i % 5 == 0 ) $s += "Buzz";
		if( IsEmpty( $s ) ) $s = ToString( $i );
		Log( $s );
	}
}
```  

If you use a array, it can description it as follows.
```php
/*
 * エラトステネスの篩
 * 素数を求めるアルゴリズム
 */
function main(){
	$buf as array[5000];
	for( $i = 2 ; $i < 100 ; $i+=1 ){
		if( $buf[$i] ){
			continue;
		}
		for( $j = 2 ; $j < 50 ; $j += 1 ){
			if( $i * $j > 100 ){
				break;
			}
			$buf[$i * $j] = 1;
		}
		Log( ToString( $i ) );
	}
}
```
  
If you use a struct, it can description it as following.
```php
/*
 * How to decl struct
 * if you access to field, it need to use `$this`
 * $this.member
 */
struct Vector2 {
	$x;
	$y;
	
	function Func(){
		return $this.x * $this.x;
	}
  
	function Func2(){
		return $this.Func() * 3;
	}
}

function main(){
	$vect as Vector2; // $var as StructType
	$vAry as Vector2[10]; // if you use a array
	$vect.x = 100;
	$vect.y = 200;
	Log("x:" + ToString($vect.x));
	Log("y:" + ToString($vect.y));
	Log("func:" + ToString($vect.Func()));
	Log("func:" + ToString($vect.Func2()));
	$vAry[0].x = 230;
	$vAry[0].y = 250;
	$vAry[1].x = 330;
	$vAry[1].y = 350;
	$vAry[2].x = 430;
	$vAry[2].y = 450;
  
	for( $i = 0 ; $i < 10 ; $i+=1 ) {
		Log("index:" + ToString($i) + "x:" + ToString($vAry[$i].x) + ",y:" + ToString($vAry[$i].y));
	}
}
```
  

  

  

  

  
This engine can generate assemble code log, it feels like as follows.
```
func main
stack frame 23
00000000:       mov R0 , 100
00000012:       mov L[0][0] , R0
00000032:       mov R0 , L[0][0]
00000052:       mov R0 , 200
00000064:       mov L[0][1] , R0
00000084:       mov R0 , L[0][1]
00000104:       mov R0 , x:
00000111:       mov R1 , L[0][0]
00000131:      push R1
00000134:        st 1
00000136:      call 1(built in function)
00000141:       mov R1 , R0
00000146:        ld 1
00000148:       add R0 , R1
00000153:      push R0
00000156:        st 0
00000158:      call 0(built in function)
00000163:       mov R0 , R0
00000168:        ld 0
00000170:       mov R0 , y:
00000177:       mov R1 , L[0][1]
00000197:      push R1
00000200:        st 1
00000202:      call 1(built in function)
00000207:       mov R1 , R0
00000212:        ld 1
00000214:       add R0 , R1
00000219:      push R0
00000222:        st 0
00000224:      call 0(built in function)
00000229:       mov R0 , R0
00000234:        ld 0
00000236:       mov R0 , func:
00000246:   mov ptr R1 , L[0]
00000260:  push ptr R1
00000263:        st 1
00000265:      call 0
00000270:       mov R1 , R0
00000275:        ld 1
00000277:      push R1
00000280:        st 1
00000282:      call 1(built in function)
00000287:       mov R1 , R0
00000292:        ld 1
00000294:       add R0 , R1
00000299:      push R0
00000302:        st 0
00000304:      call 0(built in function)
00000309:       mov R0 , R0
00000314:        ld 0
00000316:       mov R0 , func:
00000326:   mov ptr R1 , L[0]
00000340:  push ptr R1
00000343:        st 1
00000345:      call 1
00000350:       mov R1 , R0
00000355:        ld 1
00000357:      push R1
00000360:        st 1
00000362:      call 1(built in function)
00000367:       mov R1 , R0
00000372:        ld 1
00000374:       add R0 , R1
00000379:      push R0
00000382:        st 0
00000384:      call 0(built in function)
00000389:       mov R0 , R0
00000394:        ld 0
00000396:       mov R0 , 0
00000408:       mov R1 , 230
00000420:       mov L[2+(2*R0)][0] , R1
00000448:       mov R0 , L[2+(2*R0)][0]
00000476:       mov R0 , 0
00000488:       mov R1 , 250
00000500:       mov L[2+(2*R0)][1] , R1
00000528:       mov R0 , L[2+(2*R0)][1]
00000556:       mov R0 , 1
00000568:       mov R1 , 330
00000580:       mov L[2+(2*R0)][0] , R1
00000608:       mov R0 , L[2+(2*R0)][0]
00000636:       mov R0 , 1
00000648:       mov R1 , 350
00000660:       mov L[2+(2*R0)][1] , R1
00000688:       mov R0 , L[2+(2*R0)][1]
00000716:       mov R0 , 2
00000728:       mov R1 , 430
00000740:       mov L[2+(2*R0)][0] , R1
00000768:       mov R0 , L[2+(2*R0)][0]
00000796:       mov R0 , 2
00000808:       mov R1 , 450
00000820:       mov L[2+(2*R0)][1] , R1
00000848:       mov R0 , L[2+(2*R0)][1]
00000876:       mov R0 , 0
00000888:       mov L[22] , R0
00000902:       mov R0 , L[22]
00000916:       mov R0 , L[22]
00000930:       mov R1 , 10
00000942:         l R0 , R1
00000947:        jz [00001214]
00000952:       mov R0 , index:
00000963:       mov R1 , L[22]
00000977:      push R1
00000980:        st 1
00000982:      call 1(built in function)
00000987:       mov R1 , R0
00000992:        ld 1
00000994:       add R0 , R1
00000999:       mov R1 , x:
00001006:       add R0 , R1
00001011:       mov R1 , L[22]
00001025:       mov R1 , L[2+(2*R1)][0]
00001053:      push R1
00001056:        st 1
00001058:      call 1(built in function)
00001063:       mov R1 , R0
00001068:        ld 1
00001070:       add R0 , R1
00001075:       mov R1 , ,y:
00001083:       add R0 , R1
00001088:       mov R1 , L[22]
00001102:       mov R1 , L[2+(2*R1)][1]
00001130:      push R1
00001133:        st 1
00001135:      call 1(built in function)
00001140:       mov R1 , R0
00001145:        ld 1
00001147:       add R0 , R1
00001152:      push R0
00001155:        st 0
00001157:      call 0(built in function)
00001162:       mov R0 , R0
00001167:        ld 0
00001169:       mov R0 , 1
00001181:       add L[22] , R0
00001195:       mov R0 , L[22]
00001209:       jmp [00000916]
00001214:       end
```

