#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <math.h>

struct estado {
  int fila;
  int columna;
  int orientacion;

  inline bool operator==(const estado& E)const {
    return (fila == E.fila) and (columna == E.columna);
  }

  inline bool operator!=(const estado& E)const {
    return fila != E.fila or columna != E.columna;
  }
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    list<Action> plan;
    bool hayplan;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Reto(const estado &origen, const estado &destino, list<Action> &plan);
    

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

    // Mis variables
    bool tengoBikini = false, tengoZapas = false;
    bool conozcoBikini = false, conozcoZapas = false, conozcoCargador = false;
    estado cargador, zapas, bikini;
    estado subdestino;
    bool haltbikini = false, haltzapas = false;
    bool desesperado = false;

    // Métodos del nivel 2
    void actualizaMapa(const estado& posicion, const vector<unsigned char>& ojos);
    bool calcularDestino(const estado& destino, estado& subdestino);

    /// Calcula la distancia Manhattan entre dos puntos
    inline unsigned distance(const estado& origen, const estado& destino)const {
      return abs(destino.fila - origen.fila) + abs(destino.columna - origen.columna);
    }

};

#endif
