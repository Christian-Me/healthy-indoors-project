/*

edit the index.html file
copy the complete html source here between

const char index_html[] PROGMEM = R"rawliteral(

    and

)rawliteral";

*/

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .card.temperature { color: #fd7e14; }
    .card.humidity { color: #1b78e2; }
    .card.pressure { color: #2a68af, }
    .card.gasResistance { color: #118a7a; }
    .card.iaq { color: #118a1b; }
    .card.staticIaq { color: #318838; }
    .card.co2Equivalent { color: #9e701b; }
    .card.breathVocEquivalent { color: #a71788; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>HEALTHY INDOORS - DASHBOARD</h3>
    <p class="packet">Reading ID: <span id="sensorId"></span></p>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> TEMPERATURE</h4><p><span class="reading"><span id="temperature"></span> &deg;C</span></p>
      </div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> HUMIDITY</h4><p><span class="reading"><span id="humidity"></span> &percnt;</span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-umbrella"></i> PRESSURE</h4><p><span class="reading"><span id="pressure"></span> hPa</span></p>
      </div>
      <div class="card gasResistance">
        <h4><i class="fas fa-compress-alt"></i> GAS RESISTANCE</h4><p><span class="reading"><span id="gasResistance"></span> &Omega;</span></p>
      </div>
      <div class="card iaq">
        <h4><i class="fas fa-poop"></i> INDEX of AIR QUALITY</h4><p><span class="reading"><span id="iaq"></span></span></p>
      </div>
      <div class="card staticIaq">
        <h4><i class="fas fa-poop"></i> static INDEX of AIR QUALITY</h4><p><span class="reading"><span id="staticIaq"></span></span></p>
      </div>
      <div class="card co2Equivalent">
        <h4><i class="fas fa-fire"></i> CO<sub>2</sub> EQUIVALENT</h4><p><span class="reading"><span id="co2Equivalent"></span> ppm</span></p>
      </div>
      <div class="card breathVocEquivalent">
        <h4><i class="fas fa-wind"></i> BREATH VOC EQUIVALENT</h4><p><span class="reading"><span id="breathVocEquivalent"></span> ppm</span></p>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("temperature").innerHTML = obj.temperature.toFixed(2);
  document.getElementById("humidity").innerHTML = obj.humidity.toFixed(2);
  document.getElementById("pressure").innerHTML = obj.pressure.toFixed(2);
  document.getElementById("gasResistance").innerHTML = obj.gasResistance.toFixed(2);
  document.getElementById("iaq").innerHTML = obj.iaq.toFixed(2);
  document.getElementById("staticIaq").innerHTML = obj.staticIaq.toFixed(2);
  document.getElementById("co2Equivalent").innerHTML = obj.co2Equivalent.toFixed(2);
  document.getElementById("breathVocEquivalent").innerHTML = obj.breathVocEquivalent.toFixed(2);
  document.getElementById("sensorId").innerHTML = obj.id;
 }, false);
}
</script>
</body>
</html>
)rawliteral";
