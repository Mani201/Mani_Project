
function setCookie(name, value, exp) {
  var date = new Date();
  date.setTime(date.getTime() + exp*24*60*60*1000);
  document.cookie = name + '=' + value + ';expires=' + date.toUTCString() + ';path=/';
}

 /* pop=event0405, 7일 뒤 만료됨 */
function getCookie(name) {
  var value = document.cookie.match('(^|;) ?' + name + '=([^;]*)(;|$)');
  return value? value[2] : null;
}

function deleteCookie(name) {
  document.cookie = name + '=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}
function on_loade() {
			// body...		
		var cookienum = {{cookies_num}};
		setCookie('code'+cookienum,{{goodsCode}},3);
		setCookie('price'+cookienum,{{goodsCode}},3);
}