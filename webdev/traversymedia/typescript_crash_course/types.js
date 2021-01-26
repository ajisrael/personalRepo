// using var instead of let bc let is intented to be used inside
// a block scope not at the highest level of the script.
var myString;
var myNum;
var myBool;
var myVar;
myString = 'Hello World!'.slice(0, 3);
myNum = 22;
myBool = true;
myVar = true;
/* Arrays can be declared either way */
// var strArr: string[];
// var numArr: number[];
// var boolArr: boolean[];
var strArr;
var numArr;
var boolArr;
strArr = ['Hello', 'World'];
numArr = [1, 2, 3];
boolArr = [true, false, false];
// Tuples
var strNumTuple;
strNumTuple = ['Hello', 4];
// Void can be null or undefined
var myVoid = null;
var myNull = undefined; // or null
var myUndefined = undefined; // or null
console.log(myString);
