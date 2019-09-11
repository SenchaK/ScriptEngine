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
  
If you use a struct, it write as following.
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

