#include <game.hpp>
#include <init.hpp>

Game::Game(Application &app){
    std::srand(std::time(nullptr));  // Initialize random seed
    app.screenHeight = SCREEN_HEIGHT;
    app.screenWidth = SCREEN_WIDTH;
    this->app = app;
    this->mIsRunning = true;
    this->mTicksCount = 0;
};

bool Game::initialize(const std::string& title){
    bool init = initSDL(app, title);
    if (init) {
        colorConfig = {
            ImVec4(114.0 / 255.0, 36.0 / 255.0, 72.0 / 255.0, 1.0f),
            ImVec4(64.0 / 255.0, 178.0 / 255.0, 88.0 / 255.0, 1.0f),
            ImVec4(85.0 / 255.0, 128.0 / 255.0, 1.0 / 255.0, 1.0f),
            true
        };
        renderConfig = {
            true,
            0,
            5,
            5,
            10
        };
        currentStateConfig = renderConfig;
        mazeComplexObject = MazeComplex(this, &colorConfig);
    }
    return init;
};


void Game::runloop(){
    while(mIsRunning){
        processInput();
        updateGame();
        generateOutput();
    }
};

void Game::processInput() {

    SDL_Event event;
    while(SDL_PollEvent(&event)){
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch(event.type){
            case SDL_QUIT:
                mIsRunning = false;
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    app.screenWidth = event.window.data1;
                    app.screenHeight = event.window.data2;
                    mazeComplexObject.resetMazeComplex();
                    mazeComplexObject.initMazeComplex();
                    break;
                default: ;
                }
            default: ;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_ESCAPE]){
        mIsRunning = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
    }
};

void Game::renderUI(bool *openFlag){

    static ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;

    ImGui::Begin("Maze Configuration", openFlag);
    ImGui::SeparatorText("Color Settings");
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.25f, 0.5f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.25f, 0.20, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.25f, 0.15f, 0.7f));
    if(ImGui::Button("Color Pulse")){
        colorConfig.colorWave = false;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.50f, 0.5f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.50f, 0.20f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.50f, 0.15f, 0.7f));
    if(ImGui::Button("Color Wave")){
        colorConfig.colorWave = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    ImGui::ColorEdit3("Color 1##4", (float*)&colorConfig.color1, base_flags);
    ImGui::ColorEdit3("Color 2##5", (float*)&colorConfig.color2, base_flags);
    ImGui::ColorEdit3("Color 3##6", (float*)&colorConfig.color3, base_flags);
    ImGui::DragFloat("Distance Coefficient", &colorConfig.distanceCoef, .1f, 0.0f, 10.0f);
    ImGui::DragFloat("Time Coefficient", &colorConfig.timeCoef, .0001f, 0.0001f, 0.20f);
    ImGui::SeparatorText("Maze Settings");
    ImGui::Checkbox("Render maze step by step?", &currentStateConfig.renderByFrame);
    ImGui::SliderInt("Cell size", &currentStateConfig.pixelSize, 10, 30);
    ImGui::SeparatorText("Room Settings");
    ImGui::SliderInt("Number of Rooms", &currentStateConfig.numRooms, 1, 10);

    if (uiRoomWidth < 2) uiRoomWidth = 2;
    if (uiRoomWidth > 20) uiRoomWidth = 20;
    if (uiRoomHeight < 2) uiRoomHeight = 2;
    if (uiRoomHeight > 20) uiRoomHeight = 20;

    ImGui::InputInt("Room Width", &currentStateConfig.roomWidth);
    ImGui::InputInt("Room Height", &currentStateConfig.roomHeight);

    if (!(currentStateConfig == renderConfig)) {
        renderConfig = currentStateConfig;
        uiNumRooms = renderConfig.numRooms;
        uiRoomWidth = renderConfig.roomWidth;
        uiRoomHeight = renderConfig.roomHeight;
        uiParamsChanged = true;
    }
    ImGui::SeparatorText("Fun Settings");
    ImGui::DragFloat("Maze Angle", &currentStateConfig.angle, 2.0f, -180.0f, 180.0f);
    if (currentStateConfig.angle < -180.0f) currentStateConfig.angle = -180.0f;
    if (currentStateConfig.angle > 180.0f) currentStateConfig.angle = 180.0f;

    if(ImGui::Button("Regenerate Maze")) {
        uiParamsChanged = true;
    }
    ImGui::End();
}

void Game::updateGame() {
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f){
        deltaTime = 0.05f;
    }
    bool windowPointer = true;
    mTicksCount = SDL_GetTicks();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    renderUI(&windowPointer);

    if (uiParamsChanged) {
        mazeComplexObject.configureRooms(uiNumRooms, uiRoomWidth, uiRoomHeight);
        mazeComplexObject.resetMazeComplex();
        mazeComplexObject.initMazeComplex();
        uiParamsChanged = false;
    }

    mazeComplexObject.updateMazeComplex(mTicksCount);
};

void Game::generateOutput(){
    ImGui::Render();
    SDL_RenderSetScale(app.renderer, app.io->DisplayFramebufferScale.x, app.io->DisplayFramebufferScale.y);

    mazeComplexObject.displayMazeComplex(mTicksCount);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), app.renderer);
    SDL_RenderPresent(app.renderer);
}

void Game::shutdown(){
    cleanup(app);
}
