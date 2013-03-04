var table = document.querySelectorAll('table.grammar')[1];
var rows = table.querySelectorAll('tr');
var row, prodKey, prodValue, cells;
var prodMap = {};
for (var i = 0, len = rows.length; i < len; i++) {
	row = rows[i];
	cells = row.querySelectorAll('td');
	if(i === 0) { // First row has a deprecated stuff
		prodKey = cells[0].innerText.split('\n')[2];
	} else {
		prodKey = cells[0].innerText;
	}
	prodValue = cells[1].innerText;
	prodMap[prodKey] = prodValue;
}
