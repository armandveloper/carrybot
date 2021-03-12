/*
  Nombre Diseño: CarryBot
  Autor: Armando Cruz Jaimes
  Materia: Aplicaciones de IoT
*/

#include <Servo.h>

class ServoMotor
{
private:
  Servo servo;
  int pin;
  int grados;
  int velocidad;

public:
  ServoMotor()
  {
    int grados = 0;
    int velocidad = 10;
  }
  void setPin(int _pin)
  {
    pin = _pin;
    servo.attach(pin);
    servo.write(grados);
  }
  void setVelocidad(int _velocidad)
  {
    velocidad = _velocidad;
  }
  void setGrados(int _grados)
  {
    grados = _grados;
  }
  void girar(int _grados)
  {
    // Girar el servo y luego asigna al atributo grados el valor recibido
    int pos = 0;
    if (grados < _grados)
    {
      for (pos = grados; pos <= _grados; pos++)
      {
        servo.write(pos);
        delay(velocidad);
      }
    }
    else
    { // Realiza el giro descendente
      for (pos = grados; pos >= _grados; pos--)
      {
        servo.write(pos);
        delay(velocidad);
      }
    }
    setGrados(_grados);
  }
};

class Ultrasonico
{
private:
  int pinTrigger;
  int pinEcho;
  long duracion;
  long distancia;
  void configurarPines()
  {
    pinMode(pinTrigger, OUTPUT);
    pinMode(pinEcho, INPUT);
  }

public:
  Ultrasonico(int trigger, int echo)
  {
    pinTrigger = trigger;
    pinEcho = echo;
    duracion = 0;
    distancia = 0;
    configurarPines();
  }
  Ultrasonico()
  {
    pinTrigger = 2;
    pinEcho = 3;
    duracion = 0;
    distancia = 0;
    configurarPines();
  }
  void leerDistancia()
  {
    // Asegura que éste apagado
    digitalWrite(pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrigger, LOW);
    duracion = pulseIn(pinEcho, HIGH);
    distancia = (duracion / 2) / 29; // escalamos el tiempo a una distancia en cm
  }
  int getDistancia()
  {
    leerDistancia();
    return distancia;
  }
};

class Cara
{
private:
  Ultrasonico ultrasonico;
  ServoMotor servo;
  int distancias[3];
  char direcciones[3] = {'L', 'F', 'R'};

public:
  void iniciar()
  {
    servo.setPin(9);
  }
  void moverDerecha()
  {
    servo.girar(0);
  }
  void moverIzquierda()
  {
    servo.girar(180);
  }
  void moverCentro()
  {
    servo.girar(90);
  }
  bool sonDistanciasIguales()
  {
    observar();
    if (distancias[0] == distancias[1] && distancias[1] == distancias[2])
    {
      return true;
    }
    return false;
  }
  char determinarDireccion()
  {
    bool sonIguales = sonDistanciasIguales();
    if (sonIguales)
      return 'B';
    int mayor = -1;
    // Un valor cualesquiera solo para que esté inicializada
    char direccion = 'N';
    for (int i = 0; i < 3; i++)
    {
      if (mayor < distancias[i])
      {
        mayor = distancias[i];
        direccion = direcciones[i];
      }
    }
    return direccion;
  }

  void observar()
  {
    moverIzquierda();
    distancias[0] = ultrasonico.getDistancia();
    Serial.print("Izq: ");
    Serial.println(distancias[0]);
    delay(800);
    moverDerecha();
    distancias[2] = ultrasonico.getDistancia();
    Serial.print("Der: ");
    Serial.println(distancias[2]);
    delay(800);
    moverCentro();
    distancias[1] = ultrasonico.getDistancia();
    Serial.print("Centro: ");
    Serial.println(distancias[1]);
  }
};

class Llanta
{
private:
  int enable;
  int pin1;
  int pin2;
  int velocidad;

public:
  Llanta(int _velocidad)
  {
    velocidad = _velocidad;
  }
  Llanta()
  {
    velocidad = 255;
  }
  void setVelocidad(int _velocidad)
  {
    // Valida que la velocidad esté en el rango de 0 - 255
    velocidad = _velocidad > 255 ? 255 : _velocidad < 0 ? 0
                                                        : _velocidad;
  }
  void setPines(int _enable, int _pin1, int _pin2)
  {
    enable = _enable;
    pin1 = _pin1;
    pin2 = _pin2;
    pinMode(enable, OUTPUT);
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
  }
  void avanzar()
  {
    analogWrite(enable, velocidad);
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
  }
  void reversa()
  {
    analogWrite(enable, velocidad);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
  }
  void detener()
  {
    analogWrite(enable, 0);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
};

class Carro
{
private:
  Llanta llantaIzq;
  Llanta llantaDer;
  Cara cara;
  void avanzar()
  {
    llantaIzq.avanzar();
    llantaDer.avanzar();
  }
  void derecha()
  {
    llantaIzq.detener();
    llantaDer.avanzar();
    delay(600);
    avanzar();
  }
  void reversa()
  {
    detener();
    llantaIzq.reversa();
    llantaDer.reversa();
    delay(600);
  }
  void izquierda()
  {
    llantaIzq.avanzar();
    llantaDer.detener();
    delay(600);
    avanzar();
  }
  void detener()
  {
    llantaIzq.detener();
    llantaDer.detener();
    delay(200);
  }

public:
  void configurar()
  {
    cara.iniciar();
    llantaIzq.setPines(5, 7, 8);
    llantaDer.setPines(6, 10, 11);
  }
  void conducir()
  {
    // Posibles valores:
    // F: avanzar
    // R: derecha
    // B: retroceder
    // L: izquierda
    char direccion = cara.determinarDireccion();
    Serial.print("Ir hacia");
    switch (direccion)
    {
    case 'F':
      Serial.println(" adelante");
      avanzar();
      break;
    case 'R':
      Serial.println(" la derecha");
      derecha();
      break;
    case 'B':
      Serial.println(" atras");
      reversa();
      break;
    case 'L':
      Serial.println(" la izquierda");
      izquierda();
      break;
    }
  }
};

Carro carro;

void setup()
{
  Serial.begin(9600);
  carro.configurar();
}

void loop()
{
  carro.conducir();
  delay(20);
}