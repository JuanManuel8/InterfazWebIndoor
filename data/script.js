// Complete project details: https://randomnerdtutorials.com/esp8266-nodemcu-web-server-websocket-sliders/

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
}

function initButton() {
    document.getElementById('button1').addEventListener('click', toggle);
    document.getElementById('button2').addEventListener('click', toggle);
    document.getElementById('button3').addEventListener('click', toggle);
    document.getElementById('button4').addEventListener('click', toggle);
    document.getElementById('button5').addEventListener('click', toggle);
    document.getElementById('button6').addEventListener('click', toggle);
  }

function getValues(){
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSliderPWM(element) {
    var sliderNumber = element.id.charAt(element.id.length-1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
    console.log(sliderValue);
    websocket.send(sliderNumber+"s"+sliderValue.toString());
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        var element = document.getElementById(key);
        var slider = document.getElementById("slider"+ (i+1).toString());
        var state = document.getElementById("state"+ (i+1).toString());
        
        if (element) {
            element.innerHTML = myObj[key];
        }

        if (slider) {
            slider.value = myObj[key];
        }
        
        if (state) {
            state.value = myObj[key];
        }

    }
}

function toggle(element){
    var stateNumber = element.id.charAt(element.id.length-1);
    var stateValue = document.getElementById(element.id).state;
    document.getElementById("state"+stateNumber).innerHTML = stateValue;
    console.log(stateValue);
    websocket.send(stateNumber+"t"+stateValue.toString());
    
}

