use("../avg.js");

var Relais = AVGPlayer.createRelais(0); 

print(Relais.getNumCards());
Relais.set(0, 0, true);
Relais.set(0, 1, false);
print(Relais.get(0, 0));
print(Relais.get(0, 1));
print(Relais.get(0, 2));

