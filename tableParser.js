var grammarTable = document.querySelectorAll('table.grammar')[1];
var rows = grammarTable.querySelectorAll('tr');
var row, prodKey, prodValue, cells;
var prodArray = [], production;
for (var i = 0, len = rows.length; i < len-1; i++) {

	row = rows[i];
	cells = row.querySelectorAll('td');
	if(i === 0) { // First row has a deprecated stuff
		prodKey = cells[0].innerText.split('\n')[2].slice(0, -2);
	} else {
		prodKey = cells[0].innerText;
		if(prodKey[prodKey.length - 2] === ' ') {
			prodKey = prodKey.slice(0, -2);
		} else {
			prodKey = prodKey.slice(0, -1);
		}
	}
	prodValue = cells[1].innerText.slice(0, -1);
	
	production = {};
	production[prodKey] = prodValue;
	prodArray.push(production);
}

var parseTable = document.querySelector('table.parse_table');
var reduces = parseTable.querySelectorAll('reduce');
var reduction, nonterms, reduKey, reduValue;
for(var i = 0, len = reduces.length; i < len; i++) {
	reduction = reduces[i];
	reduKey = reduction.innerText.split('→')[0].substr(2).slice(0,-1);
	reduValue = reduction.innerText.split('→')[1].substr(1).slice(0, -1);
	for (var j = 0, ken = prodArray.length; j < ken; j++) {
		production = prodArray[j];
		if(production[reduKey] === reduValue) {
			reduction.innerText = "r"+j;
		}
	}	
}