# Guess the Word!

## Author: 

Jiyoon Park 

## Design: 

In this two person game (Drawer and Guesser), the Guesser has to guess what the Drawer is drawing. 

## Networking: 

The game implements the client/server multiplayer by passing messages between each other. There are three messages that are being sent over the network. the control_message that send the user's input to the server, the player_state message that carries the player's position and velocity and the game_state message that carries the target and candidate words. Recieving the message happens in PlayMode.cpp, Connection.cpp, server.cpp. The server.cpp, Connection.cpp, PlayMode.cpp sends the message. 

## Screen Shot:

![Screen Shot](screenshot.png)

## How To Play:

In separate terminals:
```
./server 15466
```

```
./client localhost 15466
```

```
./client localhost 15466
```
- The first client is the Drawer and the second client is the Guesser

### Drawer

- You are given a word to draw. 

- You have to navigate around (using up, down, left right) the canvas and draw!

- Your pen will only draw when you are pressing down the __SPACE BAR__!

- Be aware, it is not easy to draw something!

### Guesser

- You have to guess what the Drawer is drawing. 

- You are given three candidate words to choose from 

- Once you catch what the drawing is of, press __1__ or __2__ or __3__ to check if your guess is correct!

- If you want to play the game again when it is over, press __r__ ( just pressing __r__ randomly will just lower your score as a penalty)

### General 

- You can only have two players. Once two players are in the game, the game shuts down if a new player tries to enter the game. 


This game was built with [NEST](NEST.md).
