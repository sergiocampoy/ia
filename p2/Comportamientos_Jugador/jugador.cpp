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

	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	/*
	cout << "Fila: " << actual.fila << endl;
	cout << "Col : " << actual.columna << endl;
	cout << "Ori : " << actual.orientacion << endl;
	*/

	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC;

	if (sensores.nivel != 4){
		if (hayplan && !plan.empty()) {
			accion = plan.front();
			plan.pop_front();
		} else {
			hayplan = pathFinding (sensores.nivel, actual, destino, plan);
		}
	}
	else {
		// Estoy en el nivel 2

		// Necesita recargar
		if (subdestino != cargador && sensores.bateria < 10*distance(actual, cargador) and
		    sensores.vida > 500 and
			 sensores.bateria < sensores.vida and conozcoCargador) {
			subdestino = cargador;
			hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
		}

		/*
		if (sensores.vida == 2999) {
			haltbikini = false;
			haltzapas = false;
		}*/

		printf("%d %d %d\n", actual.fila, actual.columna, actual.orientacion);

		// Apunta lo que ha visto en el mapa
		actualizaMapa(actual, sensores.terreno);

		// Recoge el bikini o las zapatillas
		if (sensores.terreno[0] == 'D') {
			tengoZapas = true;
			haltzapas = false;
		}
		if (sensores.terreno[0] == 'K') {
			tengoBikini = true;
			haltbikini = false;
		}

		// Ha visto el bikini
		if (haltbikini) {
			if (subdestino != bikini) {
				subdestino = bikini;
				hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
			}
		}
		if (haltzapas) {
			if (subdestino != zapas) {
				subdestino = zapas;
				hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
			}
		}

		if (hayplan && !plan.empty()) {
			// Si se está cargando, espera
			if (sensores.terreno[0] == 'X' and sensores.bateria < 2000 and sensores.bateria < sensores.vida) {
				return actIDLE;
			}
			// Si va a avanzar
			if (plan.front() == actFORWARD) {

				// Espera a que se mueva el aldeano
				if (sensores.superficie[2] == 'a') {
					return actIDLE;
				}
				
				// Si se va a chocar, calcula un nuevo plan
				if (sensores.terreno[2] == 'M' || sensores.terreno[2] == 'P' ||
				    sensores.terreno[0] != 'A' && sensores.terreno[2] == 'A' && !tengoBikini ||
					 sensores.terreno[0] != 'B' && sensores.terreno[2] == 'B' && !tengoZapas) {
					hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
				}
			}

			// Sigue el plan calculado
			accion = plan.front();
			plan.pop_front();
		} else {
			// No hay plan o lo ha terminado
			subdestino = destino;

			// Necesita recargar
			if (sensores.bateria < 5*distance(actual, cargador) and sensores.vida > 500 and
			    sensores.bateria < sensores.vida and conozcoCargador) {
				subdestino = cargador;
			}

			// Calcula el plan
			hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
		}
	}
	return accion;

		/*
		// Parte reactiva
		if (halt) {
			halt = false;
			if (!tengoZapas and conozcoZapas) {
				subdestino = zapas;
				tengoZapas = true;
			} else if (!tengoBikini and conozcoBikini) {
				subdestino = bikini;
				tengoBikini = true;
			}
			hayplan = pathFinding (sensores.nivel, actual, subdestino, plan);
		}

		// Si hay un aldeano delante, espera a que se quite
		if (sensores.superficie[2] == 'a' and plan.front() == actFORWARD) {
			return actIDLE;
		}
		// Se queda quieto mientras se carga
		if (sensores.terreno[0] == 'X' and sensores.bateria < 1000) {
			return actIDLE;
		}
		// Si se va a chocar, recalcula el plan
		bool colision =
			// sensores.superficie[2] == 'a' or
			sensores.terreno[2]    == 'M' or
			sensores.terreno[2]    == 'P';
		if (colision and plan.front() == actFORWARD) {
			calcularDestino(destino, subdestino);
			hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
		}

		// Ejecuta el plan
		if (hayplan and !plan.empty()) {
			accion = plan.front();
			plan.pop_front();
		} else {
			if (sensores.bateria <= 200) {
				subdestino = cargador;
			} else if (!tengoZapas and conozcoZapas) {
				subdestino = zapas;
				tengoZapas = true;
			} else if (!tengoBikini and conozcoBikini) {
				subdestino = bikini;
				tengoBikini = true;
			} else if (sensores.colision) {
				hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
			} else {
				calcularDestino(destino, subdestino);
				// subdestino = destino;
			}
			hayplan = pathFinding(sensores.nivel, actual, subdestino, plan);
			if (!hayplan) {
				cout << "hay un muro en medio" << endl;
				desesperado = true;
				hayplan = pathFinding(sensores.nivel, actual, destino, plan);
				desesperado = false;
			}
			if (plan.empty()) {
				accion = actTURN_R;
			}
		}
	}

  return accion;
  */
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
					return pathFinding_CostoUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
					return pathFinding_Reto(origen,destino,plan);
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

	nodo() {}
	nodo(const estado& e, const list<Action>& s = list<Action>(), int c = 0, bool z = false, bool b = false) :
	st(e), secuencia(s), costo(c), zapatillas(z), bikini(b) {}

	bool operator<(const nodo& N)const {
		return this->costo >= N.costo;
	}
};

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

  nodo current(origen);

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

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	//stack<nodo> pila;											// Lista de Abiertos
	queue<nodo> cola;

	nodo current(origen);

	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
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

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	priority_queue<nodo> cola;											// Lista de Abiertos

  nodo current(origen);

	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			hijoTurnR.costo += 1; // Reduce el número de giros que va a hacer
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			hijoTurnL.costo += 1; // Reduce el número de giros que va a hacer
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		// Calcula el costo de batería
		unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
		unsigned costo;
		switch (casilla) {
			case 'A': // Aqua
				costo = hijoForward.bikini ? 10 : 100;
				break;
			case 'B': // Bosque
				costo = hijoForward.zapatillas ? 5 : 50;
				break;
			case 'T': // Tierra
				costo = 2;
				break;
			case 'D': // Zapatillas
				hijoForward.zapatillas = true;
				costo = 1;
				break;
			case 'K': // Bikini
				hijoForward.bikini = true;
				costo = 1;
				break;
			default:	// Resto de casillas
				costo = 1;
				break;
		}
		hijoForward.costo += costo;

		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
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
/*
struct node{
	estado st;
	node *hder, *hizq, *hfrent;
	node *mejor_padre;
	unsigned g, h;
	bool zapatillas, bikini;
	Action accion;
	

	node(estado e, unsigned g = 0, unsigned h = 0) {
		st = e;
		hder = nullptr;
		hizq = nullptr;
		hfrent = nullptr;
		mejor_padre = nullptr;
		this->g = g;
		this->h = h;
		zapatillas = false;
		bikini = false;
	}

	bool operator<(const node& N)const {
		return (g+h) >= (N.g+N.h);
	}

	bool operator==(const node& N)const {
		return (st.columna     == N.st.columna    ) and
		       (st.fila        == N.st.fila       ) and
				 (st.orientacion == N.st.orientacion);
	}
};
*/
// Reto
bool ComportamientoJugador::pathFinding_Reto(const estado& origen, const estado& destino, list<Action>& plan) {
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> generados; // Lista de cerrados
	priority_queue<nodo> cola;             // Lista de abiertos

	nodo current(origen);
	cola.push(current);
	
	while (!cola.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)) {
		cola.pop();
		generados.insert(current.st);

		// costo es la valoración del nodo (f(n) = g(n) + h(n)), donde g es la distancia al destino
		// y h es el costo de batería
		current.costo += (distance(current.st, destino) / 10);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		// Añade costo a los giros (para que haga el mínimo número de giros también)
		hijoTurnR.costo++;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
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
		int costo;
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
				costo = 1;
				hijoForward.zapatillas = true;
				break;
			case 'K':
				costo = 1;
				hijoForward.bikini = true;
				break;
			case '?':
				costo = 5;
				break;
			default:
				costo = 1;
				break;
		}

		hijoForward.costo += costo;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				if (casilla != '?' or true) {
					hijoForward.secuencia.push_back(actFORWARD);
					cola.push(hijoForward);
				} else {
					if (desesperado) {
						hijoForward.secuencia.push_back(actFORWARD);
						cola.push(hijoForward);
					}
				}
			}
		}

		// Tomo el siguiente valor de la pila
		if (!cola.empty()) {
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


// Reto A*
/*
bool ComportamientoJugador::pathFinding_Reto(const estado& origen, const estado& destino, list<Action>& plan) {
	cout << "Calculando plan" << endl;
	plan.clear();
	set<node> cerrados;
	priority_queue<node> abiertos;
	node* current = new node(origen);
	// node current(origen);
	abiertos.push(*current);

	while (!abiertos.empty() and (current->st.fila != destino.fila or current->st.columna != destino.columna)) {
		abiertos.pop();
		// cout << abiertos.size() << ' ' << cerrados.size();
		// printf(" %p - %p\n", &current, current->mejor_padre);
		printf("(%d, %d)\n", current->st.fila, current->st.columna);
		cerrados.insert(*current);
		current->h = distance(current->st, destino);
		

		// Genera descendiente de girar a la derecha
		set<node>::const_iterator aux;
		node* der = new node(*current);
		der->accion = actTURN_R;
		der->mejor_padre = current; // apunta el padre
		der->g += 1; // aumenta el coste en 1
		der->h = distance(current->st, destino);
		der->st.orientacion = (der->st.orientacion + 1) % 4; // Gira
		if ((aux = cerrados.find(*der)) == cerrados.end()) {
			abiertos.push(*der);
		} else {
			// Si el padre es mejor, lo cambia
			if (aux->mejor_padre->g < der->mejor_padre->g) {
				//der->mejor_padre = aux->mejor_padre;
			}
		}

		// Genera descendiente de girar a la izquierda
		node* izq = new node(*current);
		izq->accion = actTURN_L;
		izq->mejor_padre = current; // apunta el padre
		izq->g += 1; // aumenta el coste en 1
		izq->h = distance(current->st, destino);
		izq->st.orientacion = (izq->st.orientacion + 3) % 4; // Gira
		if ((aux = cerrados.find(*izq)) == cerrados.end()) {
			abiertos.push(*izq);
		} else {
			// Si el padre es mejor, lo cambia
			if (aux->mejor_padre->g < izq->mejor_padre->g) {
				//izq->mejor_padre = aux->mejor_padre;
			}
		}

		// Genera descendiente de avanzar
		node* frente = new node(*current);
		frente->accion = actFORWARD;
		frente->mejor_padre = current;
		unsigned char casilla = mapaResultado[current->st.fila][current->st.columna];
		unsigned costo;
		switch (casilla) {
			case 'A':
				costo = frente->bikini || tengoBikini ? 10 : 100;
				break;
			case 'B':
				costo = frente->zapatillas || tengoZapas ? 5 : 50;
				break;
			case 'T':
				costo = 2;
				break;
			case 'D':
				frente->zapatillas = true;
				tengoZapas = true;
				costo = 1;
				break;
			case 'K':
				frente->bikini = true;
				tengoBikini = true;
				costo = 1;
				break;
			case '?':
				costo = 1;
				break;
			default:
				costo = 1;
				break;
		}

		frente->g += costo;
		if (!HayObstaculoDelante(frente->st)) {
			if ((aux = cerrados.find(*frente)) == cerrados.end()) {
				if (casilla != '?' or true) {
					abiertos.push(*frente);
				}
			} else {
				// Si el padre es mejor, lo cambia
				if (aux->mejor_padre->g < frente->mejor_padre->g) {
					//frente->mejor_padre = (*aux)->mejor_padre;
				}
			}
		}

		// Tomo el siguiente valor de la pila
		if (!abiertos.empty()) {
			current = &abiertos.top();
		}
	}

	cout << "Terminada la búsqueda" << endl;

	if (current->st.fila == destino.fila and current->st.columna == destino.columna){
		cout << "Cargando el plan\n";
		// plan = current->secuencia;
		//plan.push_front(current->accion);
		// node* tmp = abiertos.top().mejor_padre;
		node* tmp = abiertos.top().mejor_padre;
		printf("%p - %p\n", tmp, tmp->mejor_padre);
		while (tmp != nullptr) {
			printf("%p - %d\n", tmp, tmp->accion);
			plan.push_front(tmp->accion);
			tmp = tmp->mejor_padre;
		}
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
*/


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
				if (!tengoBikini && mapaResultado[x + mult_x*j][y + mult_y*i] == 'K') {
					haltbikini = true;
					conozcoBikini = true;
					bikini.fila = x + mult_x*j;
					bikini.columna = y + mult_y*i;
				}
				if (!tengoZapas && mapaResultado[x + mult_x*j][y + mult_y*i] == 'D') {
					haltzapas = true;
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
				if (!tengoBikini && mapaResultado[x + mult_x*i][y + mult_y*j] == 'K') {
					haltbikini = true;
					conozcoBikini = true;
					bikini.fila = x + mult_x*i;
					bikini.columna = y + mult_y*j;
				}
				if (!tengoZapas && mapaResultado[x + mult_x*i][y + mult_y*j] == 'D') {
					haltzapas = true;
					conozcoZapas = true;
					zapas.fila = x + mult_x*i;
					zapas.columna = y + mult_y*j;
				}
			}
		}
	}
}

bool ComportamientoJugador::calcularDestino(const estado& destino, estado& subdestino) {
	if (mapaResultado[destino.fila][destino.columna] != '?') {
		subdestino = destino;
		return true;
	}

	int dist, min;
	int n = mapaResultado.size();
	estado aux;
	min = INT32_MAX;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (mapaResultado[i][j] != '?') {
				dist = abs(destino.fila - i) + abs(destino.columna - j);
				if (dist < min) {
					min = dist;
					aux.fila = i;
					aux.columna = j;
				}
			}
		}
	}
	subdestino = aux;
	printf("Subdestino = (%d, %d)\n", subdestino.fila, subdestino.columna);
	return (min != INT32_MAX);
}
