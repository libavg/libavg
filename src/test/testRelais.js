use("avg.js");

var Relais = AVGPlayer.createRelais(1); 

var frame = 0;

function bla() 
{
    if (frame < 8) {
        Relais.set(0, frame, true);
    } else {
        Relais.set(0, frame-8,false);
    }
    frame++;
    if (frame > 15) {
        frame = 0;
    }
    
}

print(Relais.getNumCards());
var i;
for (i=0; i<8; i++) {
    Relais.set(0, i, true);
}
for (i=0; i<8; i++) {
    Relais.set(0, i, false);
}

AVGPlayer.setInterval(100, "bla();");

AVGPlayer.loadFile("empty.avg");
AVGPlayer.play(30);
