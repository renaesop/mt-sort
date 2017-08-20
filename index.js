/**
 * Created by fed on 2017/8/18.
 */
const sort = require('bindings')('mt_sort').sort;

const arr = new Array(10 * 1000 * 1000).fill(0).map(x => Math.random());
const arr1 = Float64Array.from(arr);
const arr2 = Float64Array.from(arr);


const t1 = Date.now();
const res = sort(arr1)
console.log('arr done: ', Date.now() - t1);

const t2 = Date.now();
const res1 = sort(arr2)
console.log('arr2 done: ', Date.now() - t2);


// console.log(res1)

// console.log(res);
module.exports = sort;