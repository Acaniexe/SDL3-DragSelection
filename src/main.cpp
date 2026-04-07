//*---goals---*\\
//1. ctrl + left click to deselect boxs (done)
//2. add single button inputs for boxs (done)
//3. improve input handling
//4. improve render pipeline


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
}; // Stores mouse position, button states, and keyboard state

InputState input;

void pressPrint() {
    std::cout << "Print" << std::endl;
}

// Drag selection state
bool isDragging = false;
float dragStartX = 0.0f;
float dragStartY = 0.0f;
SDL_FRect dragRect = {0, 0, 0, 0};

struct Box {
    SDL_FRect rect;
    bool selected = false;
    void (*OnClick) () = nullptr;
}; // Represents a selectable/movable object

// Initial test boxes
std::vector<Box> boxes = {
    {{300, 200, 100, 100}, false, pressPrint},
    {{100, 100, 80, 80}, false},
    {{500, 200, 120, 60}, false, pressPrint}
};

// Single object movement state
int movingBoxIndex = -1;
bool isMoving = false;
float grabOffsetX = 0.0f;
float grabOffsetY = 0.0f;

// Group movement state (Shift + drag)
bool isGroupMoving = false;
int prevMouseX = 0;
int prevMouseY = 0;

int main(int argc, char *argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL failed to initialise" << SDL_GetError() << std::endl;
        return 1;
    } // Initialize SDL

    SDL_Window* window = SDL_CreateWindow(
        "Testing",
        900, 600,
        SDL_WINDOW_RESIZABLE
    ); // Create window

    if (!window) {
        std::cout << "Window failed to create" << SDL_GetError() << std::endl;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (!renderer) {
        std::cout << "Renderer failed " << SDL_GetError() << std::endl;
    } // Create renderer with blending enabled

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {

            switch (event.type) {

                case SDL_EVENT_QUIT:   // Exit application
                    running = false;
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    input.mouseX = event.motion.x;
                    input.mouseY = event.motion.y;

                    // --- Group Movement (Shift + drag) ---
                    
                    if (isGroupMoving) {
                        int dx = input.mouseX - prevMouseX;
                        int dy = input.mouseY - prevMouseY;

                        // Apply movement delta to all selected boxes
                        for (auto& box : boxes) {
                            if (box.selected) {
                                box.rect.x += dx;
                                box.rect.y += dy;
                            }
                        }

                        prevMouseX = input.mouseX;
                        prevMouseY = input.mouseY;
                    }

                    // --- Single Box Movement ---
                    else if (isMoving && movingBoxIndex != -1) {
                        auto& box = boxes[movingBoxIndex];

                        box.rect.x = input.mouseX - grabOffsetX;
                        box.rect.y = input.mouseY - grabOffsetY;
                    }

                    // --- Drag Selection Box ---
                    else if (isDragging) {
                        float x = dragStartX;
                        float y = dragStartY;
                        float w = input.mouseX - dragStartX;
                        float h = input.mouseY - dragStartY;

                        // Normalise rectangle so width/height are always positive
                        if (w < 0) { x += w; w = -w; }
                        if (h < 0) { y += h; h = -h; }

                        dragRect.x = x;
                        dragRect.y = y;
                        dragRect.w = w;
                        dragRect.h = h;
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    input.mouseX = event.button.x; // Ensure accurate mouse position at click
                    input.mouseY = event.button.y;

                    if (event.button.button == SDL_BUTTON_LEFT) {
                        input.leftMouseDown = true;
                        std::cout << "Left down\n";

                        bool clickedOnBox = false;
                        bool clickedOnAnyBox = false;

                        // Check if mouse is inside any box (top-most first)
                        for (int i = (int)boxes.size() -1; i>= 0; --i) {
                            auto& box = boxes[i];

                            bool inside =
                                input.mouseX >= box.rect.x &&
                                input.mouseX <= box.rect.x + box.rect.w &&
                                input.mouseY >= box.rect.y &&
                                input.mouseY <= box.rect.y + box.rect.h;

                            if (inside) {
                                clickedOnAnyBox = true;

                                bool shiftHeld = input.keysDown[SDL_SCANCODE_LSHIFT];
                                bool ctrlHeld =  input.keysDown[SDL_SCANCODE_LCTRL];

                                if (!isDragging && !box.selected && !shiftHeld && !ctrlHeld) {
                                    //click event for buttons
                                    box.OnClick();
                                }

                                if (ctrlHeld) {
                                    //toggle select
                                    box.selected = !box.selected;
                                    clickedOnBox = true;
                                    break;
                                }

                                if (box.selected) {
                                    
                                    if (shiftHeld) {
                                        // Start group movement
                                        isGroupMoving = true;
                                        prevMouseX = input.mouseX;
                                        prevMouseY = input.mouseY;
                                    } else {
                                        // Start single box movement
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

                        // Start drag selection if clicking empty space
                        if (!clickedOnAnyBox) {
                            isDragging = true;

                            dragStartX = input.mouseX;
                            dragStartY = input.mouseY;

                            dragRect = {dragStartX, dragStartY, 0, 0};
                        }
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) { // Right click press
                        input.rightMouseDown = true;
                        std::cout << "Right down\n";
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    input.mouseX = event.button.x; // Ensure accurate mouse position at release
                    input.mouseY = event.button.y;

                    if (event.button.button == SDL_BUTTON_LEFT) {
                        input.leftMouseDown = false;

                        // Reset movement states
                        isMoving = false;
                        isGroupMoving = false;
                        movingBoxIndex = -1;
                        std::cout << "Left up\n";

                        // Apply selection if dragging
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
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) { // Right click release
                        input.rightMouseDown = false;
                        std::cout << "Right up\n";
                    }
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (!event.key.repeat) {
                        input.keysDown[event.key.scancode] = true; // Track key press (e.g. Shift)
                    }
                    break;

                case SDL_EVENT_KEY_UP:
                    input.keysDown[event.key.scancode] = false; // Track key release
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 240, 255); // Background (Lavender)
        SDL_RenderClear(renderer);

        // Render all boxes (colour indicates selection state)
        for (auto& box : boxes) {
            if (box.selected) {
                SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }
            SDL_RenderFillRect(renderer, &box.rect);
        }

        // Render drag selection rectangle
        SDL_SetRenderDrawColor(renderer, 105, 105, 105, 80);
        if (isDragging) {
            SDL_RenderFillRect(renderer, &dragRect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}