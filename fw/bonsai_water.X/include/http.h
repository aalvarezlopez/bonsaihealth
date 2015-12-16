/*/char main_page[]={"<html>"\
	"<head>"\
	"<style media=\"screen\" type=\"text/css\">"
	"body {text-align:center ; background-color: #BAD3B9}"\
	"</style>"\
	"<TITLE>AUTOMATIC BONSAI CONTROLLER</TITLE>"\
	"</head><body>"\
	"<h1>BONSAI ENVIROMENT STATUS!</h1>"\
	"<h2>Temperature=21 ºC</h2>"\
	"<h2>Light=70\%</h2>"\
	"<h2>Soil humedity=20\%</h2>"\
	"</body> </html>"};
*/

#define MAX_DATA_HIST_LEN 2024
#define MAX_SNAPSHOT_LEN 128
char head_page[]={"<html><head>"
	"<style media=\"screen\" type=\"text/css\">"
	"body {text-align:center ; background-color: #BAD3B9}"\
	"</style>"\
	"<script type=\"text/javascript\""\
	"src=\"https://www.google.com/jsapi?autoload={'modules':[{'name':'visualization',"\
	"'version':'1','packages':['corechart']}]}\"></script><script type=\"text/javascript\">"
	"google.setOnLoadCallback(drawChart);function drawChart(){"\
	"var data = google.visualization.arrayToDataTable("\
	"[['Time','Temperature','Light','Soil hum.','Pump State'],"};
char data_hist_page[MAX_DATA_HIST_LEN];
#define DATA_HIST_MACRO	"['%02d:%02d:%02d',%d,%d,%d,%d]"
char body_page[]={"]);var options = { title: 'History',curveType:'function',"\
	"legend:{position:'bottom'}};"\
	"var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));"\
	"chart.draw(data, options);}</script></head><body>"\
	"<h1>BONSAI ENVIROMENT STATUS!</h1>"};
char snapshot_page[MAX_SNAPSHOT_LEN];
#define SNAPSHOT_MACRO	"<h2> TIME: %02d:%02d:%02d</h2>"\
	"<h2>Temperature=%dºC</h2>"\
	"<h2>Light=%d\%</h2>"\
	"<h2>Soil humedity=%d\%</h2>"\
	"<h2>Pump state=%d\%</h2>"
char bot_page[]={
	"<div id=\"curve_chart\" style=\"width: 900px; height: 500px\"></div></body></html>"};
