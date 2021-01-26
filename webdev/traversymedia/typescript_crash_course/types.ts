// using var instead of let bc let is intented to be used inside
// a block scope not at the highest level of the script.
var myString: string;
var myNum: number;
var myBool: boolean;
var myVar: any;

myString = 'Hello World!'.slice(0, 3);
myNum = 22;
myBool = true;
myVar = true;

/* Arrays can be declared either way */
// var strArr: string[];
// var numArr: number[];
// var boolArr: boolean[];

var strArr: Array<string>;
var numArr: Array<number>;
var boolArr: Array<boolean>;

strArr = ['Hello', 'World'];
numArr = [1, 2, 3];
boolArr = [true, false, false];

// Tuples
var strNumTuple: [string, number];
strNumTuple = ['Hello', 4];

// Void can be null or undefined
var myVoid: void = null;
var myNull: null = undefined; // or null
var myUndefined: undefined = undefined; // or null

console.log(myString);
