//
// Created by eduar on 24/06/2025.
//

#ifndef FSM_H
#define FSM_H

class Enemy;
class Player;

// definir quais sao os estados
// Patrulhar -> fica andando de um lado pro outro
// Perseguiçao -> vai atras do jogador
// Foge -> se a vida dele tiver acabando ou se o jogador saiu da zona de ataque dele(As vezes a gente coloca ele pra poder recuperar a vida)
//Morrer -> acabou a vida

// quais sao as transições entre os estados
//Patrulhar -> perseguir : se o jogador entrar no raio dele
//Perseguir -> fugir: se o jogador sair do raio dele

// e as ações que serão feitas em cada estado


//SERVE PRA DEFINIR APENAS UMA INTERFACE, entao nem precisa de cpp
class State {

    public:
    virtual ~State() = default;
    // Vai obrigar as classes que vao chamar essa classe a implementar esse metodo

    // Funçao de preparaçao
    virtual void Enter(Enemy* enemy) = 0;

    // A função de "ação principal", executada repetidamente
    virtual void Exit(Enemy* enemy) = 0;

    // A função de "limpeza"
    virtual void Update(Enemy* enemy, float dt) = 0;
};

#endif //FSM_H
