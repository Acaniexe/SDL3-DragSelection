#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <unordered_map>
#include <vector>

struct InputState {
    int mouseX, mouseY;
    bool leftMouseDown = false;
    bool rightMouseDown = false;

    std::unordered_map<SDL_Scancode, bool> keysDown;
}; // Input system for mouse

InputState input;

bool isDragging = false;              //
float dragStartX = 0.0f;              // This is all for the drawing 
float dragStartY = 0.0f;              // of the drag window selector
SDL_FRect dragRect = {0, 0, 0, 0};    //

struct Box {
    SDL_FRect rect;
    bool selected = false;
};

std::vector<Box> boxes = {
    {{300, 200, 100, 100}, false},
    {{100, 100, 80, 80}, false},
    {{500, 200, 120, 60}, false}
};
int movingBoxIndex = -1;      
bool isMoving = false;
float grabOffsetX = 0.0f;
float grabOffsetY = 0.0f;

bool isGroupMoving = false;
int prevMouseX = 0;
int prevMouseY = 0;

int main(int argc, char *argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL failed to initialise" << SDL_GetError() << std::endl;
        return 1;
    } // Start SDL

    SDL_Window* window = SDL_CreateWindow(
        "Testing",
        900, 600,
        SDL_WINDOW_RESIZABLE
    ); // Create window

    if (!window) {
        std::cout << "Window failed to create" << SDL_GetError() << std::endl;
    } // Checks if window was successfully created

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (!renderer) {
        std::cout << "Renderer failed " << SDL_GetError() << std::endl;
    } // Creates Renderer and sets render mode to blend

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {

            switch (event.type) {

                case SDL_EVENT_QUIT:   //For quitting
                    running = false;
                    break;

                case SDL_EVENT_MOUSE_MOTION:  //For mouse movement
                    input.mouseX = event.motion.x;
                    input.mouseY = event.motion.y;
                    if (isGroupMoving) {
                        int dx = input.mouseX - prevMouseX;
                        int dy = input.mouseY - prevMouseY;

                        for (auto& box : boxes) {
                            if (box.selected) {
                                box.rect.x += dx;
                                box.rect.y += dy;
                            }
                        }

                        prevMouseX = input.mouseX;
                        prevMouseY = input.mouseY;
                    }
                    else if (isMoving && movingBoxIndex != -1) {
                        auto& box = boxes[movingBoxIndex];

                        box.rect.x = input.mouseX - grabOffsetX;
                        box.rect.y = input.mouseY - grabOffsetY;
                    }
                    else if (isDragging) {
                        float x = dragStartX;
                        float y = dragStartY;
                        float w = input.mouseX - dragStartX;
                        float h = input.mouseY - dragStartY;

                        if (w < 0) {x += w; w = -w;}
                        if (h < 0) {y += h; h = -h;}

                        dragRect.x = x;
                        dragRect.y = y;
                        dragRect.w = w;
                        dragRect.h = h;
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:  //For mouse button down
                    input.mouseX = event.button.x;
                    input.mouseY = event.button.y;
                    
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        input.leftMouseDown = true;
                        std::cout << "Left down\n";

                        bool clickedOnBox = false;
                        bool clickedOnAnyBox = false;
                        
                        for (int i = (int)boxes.size() -1; i>= 0; --i) {
                            auto& box = boxes[i];

                            bool inside =
                                input.mouseX >= box.rect.x &&
                                input.mouseX <= box.rect.x + box.rect.w &&
                                input.mouseY >= box.rect.y &&
                                input.mouseY <= box.rect.y + box.rect.h;

                            if (inside) {
                                clickedOnAnyBox = true;

                                if (box.selected) { 
                                    bool shiftHeld = input.keysDown[SDL_SCANCODE_LSHIFT];

                                    if (shiftHeld) {
                                        //Group move
                                        isGroupMoving = true;
                                        prevMouseX = input.mouseX;
                                        prevMouseY = input.mouseY;
                                    } else {
                                        //single move
                                        isMoving = true;
                                        movingBoxIndex = i;

                                        grabOffsetX = input.mouseX - box.rect.x;
                                        grabOffsetY = input.mouseY - box.rect.y;
                                    }
                                    clickedOnBox = true;
                                    break;
                                }
                            }
                        } 
                        if (!clickedOnAnyBox) {
                            isDragging = true;

                            dragStartX = input.mouseX;
                            dragStartY = input.mouseY;

                            dragRect = {dragStartX, dragStartY, 0, 0};
                        }
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        input.rightMouseDown = true;
                        std::cout << "Right down\n";
                    }
                    break;
                    
                case SDL_EVENT_MOUSE_BUTTON_UP:  //For mouse button up
                    input.mouseX = event.button.x;
                    input.mouseY = event.button.y;

                    if (event.button.button == SDL_BUTTON_LEFT) {
                        input.leftMouseDown = false;
                        
                        isMoving = false;
                        isGroupMoving = false;
                        movingBoxIndex = -1;
                        std::cout << "Left up\n";

                        if (isDragging){
                            for (auto& box : boxes) {
                                bool overlap = 
                                    box.rect.x < dragRect.x + dragRect.w &&
                                    box.rect.x + box.rect.w > dragRect.x &&
                                    box.rect.y < dragRect.y + dragRect.h &&
                                    box.rect.y + box.rect.h > dragRect.y;
                                box.selected = overlap;
                            }
                        }

                        isDragging = false;
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        input.rightMouseDown = false;
                        std::cout << "Right up\n";
                    }
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (!event.key.repeat) {
                        input.keysDown[event.key.scancode] = true;
                    }
                    break;
                    
                case SDL_EVENT_KEY_UP:
                    input.keysDown[event.key.scancode] = false;
                    break;
                    
            }   

        }

        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); //Colour for window
        SDL_RenderClear(renderer); //clears renderer

        for (auto& box : boxes) { //For rendering multiple boxes and sets color per state
            if (box.selected) {
                SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }
            SDL_RenderFillRect(renderer, &box.rect);
        }

        SDL_SetRenderDrawColor(renderer, 5, 5, 5, 80); //Colour for drag box
        if (isDragging) {
            SDL_RenderFillRect(renderer, &dragRect); //Renders only if dragging
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}