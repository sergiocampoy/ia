#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;
	// Estoy en el nivel 1

	// Actualiza la posición actual
	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	// Muestra la posición actual
	cout << "Fila: " << actual.fila << endl;
	cout << "Col : " << actual.columna << endl;
	cout << "Ori : " << actual.orientacion << endl;

	// Actualiza la posición de destino
	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC;

	if (sensores.nivel != 4){
		if (hayplan && !plan.empty()) {
			accion = plan.front();
			plan.erase(plan.begin());
		} else {
			hayplan = pathFinding(sensores.nivel, actual, destino, plan);
		}
	}
	else {
		// Estoy en el nivel 2
		accion = actIDLE;

		actualizaMapa(actual, sensores.terreno);
		char p[100];
		sprintf(p, "%03d", 10);

		if (sensores.superficie[2] == 'a' and plan.front() == actFORWARD) {
			return actIDLE;
		}
		if (sensores.terreno[0] == 'X' && (sensores.bateria < 1000)) {
			return actIDLE;
		}
		if (hayplan && !plan.empty()) {
			accion = plan.front();
			plan.erase(plan.begin());
		} else {
			if (sensores.bateria <= 200) {
				subdestino = cargador;
			} else if (!tengoZapas and conozcoZapas) {
				subdestino = zapas;
				tengoZapas = true;
			} else if (!tengoBikini and conozcoBikini) {
				subdestino = bikini;
				tengoBikini = true;
			} else {
				calcularDestino(destino, subdestino);
			}
			hayplan = pathFinding2(actual, subdestino, plan);
			if (plan.empty()) {
				accion = actTURN_R;
			}
		}
	}

  return accion;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen, destino, plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
						return pathFinding_CostoUniforme(origen, destino, plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						// Incluir aqui la llamada al algoritmo de búsqueda usado en el nivel 2
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}




struct nodo{
	estado st;
	list<Action> secuencia;
	// Variables para la búsqueda de Costo Uniforme
	int costo;
	bool zapatillas, bikini;
};

bool operator<(const nodo& a, const nodo& b) {
	return a.costo >= b.costo;
}

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}




// Implementación de la búsqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	list<nodo> lista;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	//pila.push(current);
	lista.push_back(current);

  while (!lista.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		//pila.pop();
		lista.pop_front();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			//pila.push(hijoTurnR);
			lista.push_back(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			//pila.push(hijoTurnL);
			lista.push_back(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				//pila.push(hijoForward);
				lista.push_back(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!lista.empty()){
			current = lista.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}



// Implementación de la búsqueda en coste uniforme.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	priority_queue<nodo> cola;						// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.costo = 0;
	current.bikini = false;
	current.zapatillas = false;

	//pila.push(current);
	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		//pila.pop();
		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		// Añade costo a los giros (para que haga el mínimo número de giros también)
		hijoTurnR.costo++;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			//pila.push(hijoTurnR);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		// Añade costo a los giros (para que haga el mínimo número de giros también)
		hijoTurnL.costo++;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			//pila.push(hijoTurnL);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		// Calcula el costo de batería
		unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
		unsigned costo;
		switch (casilla) {
			case 'A':
				costo = hijoForward.bikini ? 10 : 100;
				break;
			case 'B':
				costo = hijoForward.zapatillas ? 5 : 50;
				break;
			case 'T':
				costo = 2;
				break;
			case 'D':
				hijoForward.zapatillas = true;
				costo = 1;
				break;
			case 'K':
				hijoForward.zapatillas = true;
				costo = 1;
				break;
			default:
				costo = 1;
				break;
		}
		hijoForward.costo = current.costo + costo;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				//pila.push(hijoForward);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!cola.empty()){
			current = cola.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}






// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}


// Funciones del nivel 2
void ComportamientoJugador::actualizaMapa(const estado& posicion, const vector<unsigned char>& ojos) {
	int mult_x, mult_y;
	switch (posicion.orientacion) {
		case 0: mult_x = -1; mult_y =  1; break;
		case 1: mult_x =  1; mult_y =  1; break;
		case 2: mult_x =  1; mult_y = -1; break;
		case 3: mult_x = -1; mult_y = -1; break;
	}

	int x = posicion.fila, y = posicion.columna;
	mapaResultado[x][y] = ojos[0];

	if (posicion.orientacion == 1 || posicion.orientacion == 3) {
		for (int i = 1; i <= 3; i++) {
			for (int j = -i; j <= i; j++) {
				mapaResultado[x + mult_x*j][y + mult_y*i] = ojos[i*i+i + j];

				if (!conozcoCargador && mapaResultado[x + mult_x*j][y + mult_y*i] == 'X') {
					conozcoCargador = true;
					cargador.fila = x + mult_x*j;
					cargador.columna = y + mult_y*i;
				}
				if (!conozcoBikini && mapaResultado[x + mult_x*j][y + mult_y*i] == 'K') {
					conozcoBikini = true;
					bikini.fila = x + mult_x*j;
					bikini.columna = y + mult_y*i;
				}
				if (!conozcoZapas && mapaResultado[x + mult_x*j][y + mult_y*i] == 'D') {
					conozcoZapas = true;
					zapas.fila = x + mult_x*j;
					zapas.columna = y + mult_y*i;
				}
			}
		}
	} else {
		for (int i = 1; i <= 3; i++) {
			for (int j = -i; j <= i; j++) {
				mapaResultado[x + mult_x*i][y + mult_y*j] = ojos[i*i+i + j];

				if (!conozcoCargador && mapaResultado[x + mult_x*i][y + mult_y*j] == 'X') {
					conozcoCargador = true;
					cargador.fila = x + mult_x*i;
					cargador.columna = y + mult_y*j;
				}
				if (!conozcoBikini && mapaResultado[x + mult_x*i][y + mult_y*j] == 'K') {
					conozcoBikini = true;
					bikini.fila = x + mult_x*i;
					bikini.columna = y + mult_y*j;
				}
				if (!conozcoZapas && mapaResultado[x + mult_x*i][y + mult_y*j] == 'D') {
					conozcoZapas = true;
					zapas.fila = x + mult_x*i;
					zapas.columna = y + mult_y*j;
				}
			}
		}
	}
}

bool ComportamientoJugador::calcularDestino(const estado& destino, estado& subdestino) {
	// mapaResultado[current.st.fila][current.st.columna]
	int dist, min;
	int n = mapaResultado.size();
	estado tmp;
	min = INT32_MAX;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (mapaResultado[i][j] != '?') {
				dist = abs(destino.fila - i) + abs(destino.columna - j);
				if (dist < min) {
					min = dist;
					subdestino.fila = i;
					subdestino.columna = j;

				}
			}
		}
	}
	return (min != INFINITY);
}


// Implementación de la búsqueda en coste uniforme.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding2(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	priority_queue<nodo> cola;						// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.costo = 0;
	current.bikini = false;
	current.zapatillas = false;

	//pila.push(current);
	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		//pila.pop();
		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		// Añade costo a los giros (para que haga el mínimo número de giros también)
		hijoTurnR.costo++;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			//pila.push(hijoTurnR);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		// Añade costo a los giros (para que haga el mínimo número de giros también)
		hijoTurnL.costo++;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			//pila.push(hijoTurnL);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		// Calcula el costo de batería
		unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
		unsigned costo;
		switch (casilla) {
			case 'A':
				costo = hijoForward.bikini || tengoBikini ? 10 : 100;
				break;
			case 'B':
				costo = hijoForward.zapatillas || tengoZapas ? 5 : 50;
				break;
			case 'T':
				costo = 2;
				break;
			case 'D':
				hijoForward.zapatillas = true;
				tengoZapas = true;
				costo = 1;
				break;
			case 'K':
				hijoForward.bikini = true;
				tengoBikini = true;
				costo = 1;
				break;
			default:
				costo = 1;
				break;
		}
		hijoForward.costo = current.costo + costo;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				if (casilla != '?') {
					hijoForward.secuencia.push_back(actFORWARD);
					//pila.push(hijoForward);
					cola.push(hijoForward);
				}
			}
		}

		// Tomo el siguiente valor de la pila
		if (!cola.empty()){
			current = cola.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

int ComportamientoJugador::distance(const estado& origen, const estado& destino) {
	return abs(destino.fila - origen.fila) + abs(destino.columna - origen.columna);
}