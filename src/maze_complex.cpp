#include <maze_complex.hpp>
#include <game.hpp>
#include <utils.hpp>

/**
 * MazeComplex Default Constructor
 * @brief Constructor for MazeComplex

 * @memberof MazeComplex
 */
MazeComplex::MazeComplex(){
    this->mazeComplete = false;
    this->pixelSize = 10;
    this->background = {0x10, 0x10, 0x10, 255};
    this->wallColor = {255, 255, 255, 255};
    this->configRenderMazePerFrame = true;
}

/**
 * MazeComplex Constructor
 * @brief Constructor, used in @game.cpp
 * @param game - Game Object, Dependency injection 
 * @param config - ColorConfig object, configures maze color options
 * @memberof MazeComplex
 */
MazeComplex::MazeComplex(
    Game* game,
    ColorConfig* config
){
    this->mazeComplete = false;
    this->background = {0x10, 0x10, 0x10, 255};
    this->wallColor = {255, 255, 255, 255};
    this->mazeColorConfig = config;
    this->game = game;
    this->pixelSize = game->renderConfig.pixelSize;
    initMazeComplex();
}

/**
 * @name initMazeComplex
 * @brief Initializes the mazeComplex object. This consists of:
 * 1. Set up maze grid, dividing screen by pixel size to get number of cells in x and y directions
 * 2. Create structures that hold CELL structure types for each cell
 * 3. Determine if rooms have been added via configuration, if so, call addRoom
 * 4. Create starting cell, set startX and startY
 * 5. Calculate maxDistance as manhatten distance from start to a corner
 * 6. Initialize frontier with neighbors of starting cell
 * @memberof MazeComplex
 */
void MazeComplex::initMazeComplex(){
    this->pixelSize = game->renderConfig.pixelSize;
    this->mazeComplete = false;
    this->numCellX = game->app.screenWidth / pixelSize;
    this->numCellY = game->app.screenHeight / pixelSize;
    for(int i =0; i < numCellX * numCellY; i++){
        int gridX = i % numCellX;
        int gridY = i / numCellX;
        auto cell = std::make_shared<MazeStructure>(1, 1, gridX, gridY, numCellX);
        maze.emplace_back(gridX, gridY, i, cell);
    }

    int start = std::rand() % (numCellX * numCellY);

    for(int i = 0; i <configNumRooms; i++){

        addRoom(configRoomWidth, configRoomHeight);
    }
    maze[start].visited = true;
    maze[start].generationTime = 0;
    startX = maze[start].gridX;
    startY = maze[start].gridY;

    int maxX = std::max(startX, numCellX - startX);
    int maxY = std::max(startY, numCellY - startY);
    maxDistance = maxX + maxY;
    std::vector<int> neighbors = getNeighbors(maze[start], false);
    frontier.insert(neighbors.begin(), neighbors.end());
    //frontier.insert(frontier.end(), neighbors.begin(), neighbors.end());

}

/**
 * @name configureRooms
 * @brief Configures the number of rooms, width, and height
 * @param numRooms - Integer, number of rooms
 * @param width - Integer, width of each room
 * @param height - Integer, height of each room
 * @memberof MazeComplex
 */
void MazeComplex::configureRooms(int numRooms, int width, int height){
    configNumRooms = numRooms;
    configRoomWidth = width;
    configRoomHeight = height;
}


/**
 * @name resetMazeComplex
 * @brief Resets the mazeComplex object. This consists of:
 * 1. Clearing the maze and frontier vectors
 * 2. Normally we'd have to worry abou all the MazeStructure objects, but we use shared_ptrs
 * so they'll be cleaned up when the maze is cleared. (MazeElement with pointers to MazeStructure)
 * @memberof MazeComplex
 */
void MazeComplex::resetMazeComplex(){
    // Use a set to track unique pointers to avoid double deletion
    maze.clear();
    frontier.clear();
}

/**
 * @name generateCompleteMaze
 * @brief Generate a maze using Prim's algorithm. Simply do the following via a lookahead:
 * 1. Choose a cell in the frontier (at the beginning we only can choose the starting cell)
 * 2. (Frontier means its a neighbor to a visited cell) (Our lookahead)
 * 3. Mark it as visited, set distance, gen time for visualization options
 * 4. Next, we do two things (a lookback, a lookahead):
 *    1. Because we don't store the cell we choose last time, we need to find the visited neighbor connected to my frontier choice
 *       That's what chooseWallCandidate does. You can think of it as a look back. Once we find one, we "remove" a wall between them. 
 *       That wall removal is shown by a block of color. If it's not visited, it shows as a default grey black. If it is visited, 
 *       we give it a nice color gradient effect. 
 *    2. Once we've removed the wall, we get the unvisited neighbors of our frontier choice (lookahead) and add them to the frontier
 *    3. Move on until frontier is empty (no more candidates left, we're done.)
 * @memberof MazeComplex
 **/
void MazeComplex::generateCompleteMaze(){

    while(!frontier.empty()){
        int randomIndex = std::rand() % frontier.size();
        auto it = frontier.begin();
        std::advance(it, randomIndex);
        int cell = *it;
        frontier.erase(it);
        
        
        maze[cell].visited = true;
        maze[cell].generationTime = 0;  // Set to 0 for instant generation
        
        int gridX = maze[cell].gridX;
        int gridY = maze[cell].gridY;
        int distance = calculateDistance(startX, startY, gridX, gridY);
        maze[cell].distance = distance;
        
        chooseWallCandidate(cell);
        
        std::vector<int> unVisited = getNeighbors(maze[cell], false);
        for(int neighbor : unVisited) {
            frontier.insert(neighbor);
            // if(std::find(frontier.begin(), frontier.end(), neighbor) == frontier.end()) {
            //     frontier.push_back(neighbor);
            // }
        }
    }
    mazeComplete = true;
}

/**
 * @name updateMazeComplex
 * @brief This is our logic rendering loop for MazeComplex. We do the following:
 * 1. If the maze is not complete and frontier is not empty, we need to render the maze!
 *    1. if configRenderMazePerFrame is true, we'll render the maze per frame. Basically we tie our lookahead to the game loop. 
 *       So if we're going 60 fps, that means we're firing our lookahead 60 times a second, so 60 map cells get rendered.
 *    2. If configRenderMazePerFrame is false, we render the whole maze in one frame via generateCompleteMaze.
 * 2. Frontier is empty, we're almost done. Have to set mazeCompletionTime so that, if we're using the one frame render (configRenderMazePerFrame is false),
 *    we need to set a timedelta that allow us to see the finished maze before its reset. 
 * 3. Maze is actually complete. We reset the maze if we're using the one cell per frame render. Otherwise, we wait for mazeDisplayTime milliseconds before resetting. 
 * @param currentTime - Integer, current time in milliseconds
 * @memberof MazeComplex
 */
void MazeComplex::updateMazeComplex(Uint32 currentTime){
    if (!mazeComplete && !frontier.empty()){
        if (game->renderConfig.renderByFrame){
            lookahead(currentTime);
        }else {
            generateCompleteMaze();
        }
    }

    if (frontier.empty()){
        mazeComplete = true;
        if(mazeCompletionTime == 0){
            mazeCompletionTime = currentTime;
        }
    }
    if (mazeComplete){
        if (game->renderConfig.renderByFrame){
            resetMazeComplex();
            initMazeComplex();
        }else if (currentTime - mazeCompletionTime > mazeDisplayTime){
            resetMazeComplex();
            initMazeComplex();
            mazeCompletionTime = 0;
        }
    }
}

/**
 * @name generateColor
 * @brief Generates a color based on the distance and time. 
 * Note, some params here are outdated, like mazeElemTime. 
 * We apply a sine function based on distance and time. If distance is un-normalized, it creates a color wave. 
 * Why? The distance is how far our cell is from the center. The sine curve will alternate through the colors
 * as the distance increases. If the distance is significant, then the time it will take to travel from center to edge has an effect. 
 * If we use unnormalize distance, we bind our distance between 0 and 1. So its effect becomes negligible, and 
 * every cell has about the same color. 
 * @param distance - Integer, distance from center
 * @param time - Unsigned integer, current time in milliseconds
 * @memberof MazeComplex
 */
SDL_Color MazeComplex::generateColor(int distance, Uint32 time){
    SDL_Color color;
    if(!mazeColorConfig->colorWave){
        float normal_distance = (float) distance / (float) maxDistance;
        // Clamp to prevent values > 1.0 that cause color inversion
        distance = (int) normal_distance;
        //distance = std::min(normal_distance, 1.0f);
    }
    SDL_Color first_color = ImVec4ToSDLColor(mazeColorConfig->color1);
    SDL_Color second_color = ImVec4ToSDLColor(mazeColorConfig->color2);
    SDL_Color third_color = ImVec4ToSDLColor(mazeColorConfig->color3);
    float wave = sin((distance * mazeColorConfig->distanceCoef) -((float) time * mazeColorConfig->timeCoef)) * 0.5f + 0.5f;
    float pulse = 1.0;

    float intensity =  wave * pulse;
    if (intensity < .33) {
        color = first_color;
    } else if(intensity < .66){
        color = second_color;   
    } else {
        color = third_color;
    }
    return color;
}

/**
 * @name mazeStructureNeighbors
 * @brief Helper function that iterates through a candidate cell. If its not a CELL type, then we have to 
 * check the structure's perimeter cells to find out if we can connect. A structure will have more than one
 * perimeter wall we need to check.
 * If we can, its added as a neighbor candidate and processed.
 * @param nx - vector of integers, holds neighbor candidates
 * @param neighbor - MazeElement, the candidate neighbor cell
 * @param visited - Boolean, indicates if we're looking for visited or unvisited neighbors. This helps us find
 * unvisited or visited areas of the grid.
 * @memberof MazeComplex
 */
void MazeComplex::mazeStructureNeighbors(std::vector<int> &nx, MazeElement& neighbor, bool visited){

    auto neighborStruct = neighbor.parentStructure;
    bool canConnect = true;
    if(neighborStruct->structure != CELL){
        canConnect = false;
        for (const auto& perimCell : neighborStruct->perimeterCells) {
            if (perimCell.cell == neighbor.place) {
                canConnect = true;
                break;
            }
        }
    }
    if(canConnect){
        if (visited) {
            if (neighbor.visited) {
                nx.push_back(neighbor.place);
            }
        } else {
            if (!neighbor.visited) {
                nx.push_back(neighbor.place);
            }
        }
    }

}

/**
 * @name putNeighbors
 * @brief In this function, we check each of the neighboring cells (up, down, left, right) of the current cell.
 * These neighbors are added based on the visited flag, if true, we add visited neighbors, otherwise frontier candidates.
 * We loop through the directions, and use the helper function to calculate the positions.
 * @param current_element - MazeElement, current cell we're going to check neighbors
 * @param visited
 * @return std::vector<int> - vector of integers, holds neighbor candidates
 * @memberof MazeComplex
 */
std::vector<int> MazeComplex::getNeighbors(const MazeElement& current_element, bool visited){
    std::vector<int> nx;
    int place = current_element.place;
    int gridX = current_element.gridX;
    int gridY = current_element.gridY;
    auto currentStruct = current_element.parentStructure;

    // Check all 4 directions for neighbors
    std::unordered_map<int, bool> conditions = {
        {place - 1, gridX > 0},
        {place + 1, gridX < numCellX - 1},
        {place - numCellX, gridY > 0},
        {place + numCellX, gridY < numCellY - 1}
    };
    for( auto mapElem: conditions){
        if (mapElem.second){
            int neighborPlace = mapElem.first;
            MazeElement& neighbor = maze[neighborPlace];
            mazeStructureNeighbors(nx, neighbor, visited);
        }
    }
    
    return nx;
}

/**
 * @name addRoom
 * @brief Adds a room structure to the maze based on width and height. Do an exhaustive search to
 * find a valid position for the room.
 * @param width int - Room width
 * @param height int - Room height
 */
void MazeComplex::addRoom(int width, int height){
    int attempts = 20;
    for (int i = 0; i < attempts; i++){
        // Pick a random starting position
        int randomIndex = std::rand() % maze.size();
        MazeElement elem = maze[randomIndex];
        int sX = elem.gridX;
        int sY = elem.gridY;
        
        // Check if room fits within maze boundaries
        if (sX + width > numCellX || sY + height > numCellY) {
            continue;
        }
        
        // Check if all cells in the room area are available (type CELL)
        bool canPlaceRoom = true;
        std::vector<int> roomCells;
        
        for (int y = sY; y < sY + height; y++){
            for (int x = sX; x < sX + width; x++){
                int cellIndex = y * numCellX + x;
                MazeElement& cellElem = maze[cellIndex];
                auto candidateStruct = cellElem.parentStructure;
                
                if (candidateStruct->structure != CELL) {
                    canPlaceRoom = false;
                    break;
                }
                roomCells.push_back(cellIndex);
            }
            if (!canPlaceRoom) break;
        }
        
        // If we can place the room, create the room structure and update cells
        if (canPlaceRoom) {
            // Create new room structure
            auto roomStruct = std::make_shared<MazeStructure>(width, height, sX, sY, numCellX);
            roomStruct->structure = ROOM;
            roomStruct->height = height;
            roomStruct->width = width;
            
            // Update all cells in the room to point to the new room structure
            for (int cellIndex : roomCells) {
                maze[cellIndex].parentStructure = roomStruct;
            }
            
            break;
        }
    }
}

/**
 * @name lookahead
 * @brief This represents taking a step in our maze generation algorithm. We do the following:
 *   1. Get a random cell from our frontier
 *   2. Find a connecting cell that is visited and remove the wall between them (lookback)
 *   3. Add frontier candidates from the frontier cell (lookahead)
 *   4. Repeat until frontier is empty
 * @memberof MazeComplex
 * @param currentTime
 */
void MazeComplex::lookahead(Uint32 currentTime){
    if (!frontier.empty()){
        int randomIndex = std::rand() % frontier.size();
        auto it = frontier.begin();
        std::advance(it, randomIndex);
        int cell = *it;
        frontier.erase(it);

        maze[cell].visited = true;
        maze[cell].generationTime = currentTime;
        
        // Calculate individual cell position even if part of a room
        int gridX = maze[cell].gridX;
        int gridY = maze[cell].gridY;
        int distance = calculateDistance(startX, startY, gridX, gridY);
        maze[cell].distance = distance;
        
        // Choose a visited neighbor to connect to
        chooseWallCandidate(cell);
        
        // Add unvisited neighbors to frontier (avoiding duplicates)
        std::vector<int> unVisited = getNeighbors(maze[cell], false);
        for(int neighbor : unVisited) {
            frontier.insert(neighbor);
        }
    }
}

/**
 * @name chooseWallCandidate
 * @brief This function gets the neighbors of a froniter cell and chooses a visited neighbor.
 * A frontier cell will "always" have a connected neighbor that is visited.
 * @param frontierCell
 * @memberof MazeComplex
 */
void MazeComplex::chooseWallCandidate(int frontierCell){
    MazeElement element = maze[frontierCell];
    std::vector<int> visited = getNeighbors(element, true);
    if (!visited.empty()){
        int connectVisitor = visited[std::rand() % visited.size()];
        removeWall(frontierCell, connectVisitor);
    }
}

/**
 * @name removeWall
 * @brief Get the x and y positions of two cells, and mark the perimeter walls as visited = false
 * @param cell1
 * @param cell2
 * @memberof MazeComplex
 */
void MazeComplex::removeWall(int cell1, int cell2) {
    int x1 = cell1 % numCellX, y1 = cell1 / numCellX;
    int x2 = cell2 % numCellX, y2 = cell2 / numCellX;

    // Find and remove the appropriate perimeter walls
    if(x1 == x2) { // Vertical connection
        if(y1 < y2) { // cell1 is above cell2
            // Remove south wall of cell1 and north wall of cell2
            checkCell(SOUTH, cell1, cell2);
        } else { // cell1 is below cell2
            // Remove north wall of cell1 and south wall of cell2
            checkCell(NORTH, cell1, cell2);
        }
    } else { // Horizontal connection
        if(x1 < x2) { // cell1 is left of cell2
            // Remove east wall of cell1 and west wall of cell2
            checkCell(EAST, cell1, cell2);
        } else { // cell1 is right of cell2
            // Remove west wall of cell1 and east wall of cell2
            checkCell(WEST, cell1, cell2);
        }
    }
}

/**
 * @name checkCell
 * @brief We have to mark the wall of one cell as removed and the opposite one on the other cell.
 * @param direction
 * @param one
 * @param two
 * @memberof MazeComplex
 */
void MazeComplex::checkCell(Direction direction, int one, int two){
    auto cellOne = maze[one].parentStructure;
    auto cellTwo = maze[two].parentStructure;
    
    for (auto& perim: cellOne->perimeterCells){
        if (perim.cell == one && perim.direction == direction){
            perim.visited = false;
            break;
        }
    }
    Direction oppositeDir = getOppositeDirection(direction);
    for (auto& perim: cellTwo->perimeterCells){
        if (perim.cell == two && perim.direction == oppositeDir){
            perim.visited = false;
            break;
        }
    }
}

/**
 * @name getOppositeDirection
 * @brief Given a direction, return the opposite direction
 * @param direction
 * @return Direction
 */
Direction MazeComplex::getOppositeDirection(Direction direction){
    switch(direction){
        case NORTH:
            return SOUTH;
        case SOUTH:
            return NORTH;
        case EAST:
            return WEST;
        case WEST:
            return EAST;
        default:;
    }
}

/**
 * @name displayMazeComplex
 * @brief Our rendering function. Two main loops:
 * 1. First, draw the color shift for any visited cell. The color is generated using settings from ImGui
 * 2. Second, we draw the original color of the cells, which is a white color. These are drawn as small slices.
 * 3. The background color fills in anything that's not visited that's left over. It's a dark grey black.
 * This is an expensive operation, because of all the SDL_RenderFillRect calls. We can make it more efficient
 * by creating a texture and drawing to that one directly for all calls.
 * @param currentTime
 */
void MazeComplex::displayMazeComplex(Uint32 currentTime){
    SDL_SetRenderDrawColor(game->app.renderer, background.r, background.g, background.b, background.a);
    SDL_RenderClear(game->app.renderer);
    // Draw color shift 
    for(const auto& mazeElem: maze){
        auto structure = mazeElem.parentStructure;
        if (mazeElem.visited){
            SDL_Color color = generateColor(mazeElem.distance, currentTime);

            SDL_SetRenderDrawColor(game->app.renderer, color.r * 0.8, color.g * 0.8, color.b * 0.8, color.a);
            SDL_Rect cellRect = {
                mazeElem.gridX * pixelSize,
                mazeElem.gridY * pixelSize,
                pixelSize,
                pixelSize
            };
            SDL_RenderFillRect(game->app.renderer, &cellRect);
        }
        if (structure->structure == ROOM){
            bool allperimeter_elems_covered = true;
            for(auto e: structure->perimeterCells){
                MazeElement elem = maze[e.cell];
                if (!elem.visited){
                    allperimeter_elems_covered = false;
                }
            }
            if (allperimeter_elems_covered){
                int sX = structure->startX;
                int sY = structure->startY;
                int height = structure->height;
                int width = structure->width;
                for (int y = sY; y < sY + height; y++){
                    for (int x = sX; x < sX + width; x++){
                        int cellIndex = y * numCellX + x;
                        auto cellElem = maze[cellIndex];
                        SDL_Color color = generateColor(cellElem.distance, currentTime);

                        SDL_SetRenderDrawColor(game->app.renderer, color.r * 0.8, color.g * 0.8, color.b * 0.8, color.a);
                        SDL_Rect cellRect = {
                            cellElem.gridX * pixelSize,
                            cellElem.gridY * pixelSize,
                            pixelSize,
                            pixelSize
                        };
                        SDL_RenderFillRect(game->app.renderer, &cellRect);
                    }
                }
                
            }
        }        
    }


    SDL_SetRenderDrawColor(game->app.renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);

    // Draw basic maze using perimeter cells
    for (int i = 0; i < numCellX * numCellY; i++){
        const MazeElement& element = maze[i];
        int x = element.gridX * pixelSize;
        int y = element.gridY * pixelSize;
        
        // Check perimeter cells for walls to draw
        auto structure = element.parentStructure;
        if(structure) {
            for(const auto& perimCell : structure->perimeterCells) {
                if(perimCell.cell == element.place && perimCell.visited) {
                    // Draw wall based on direction
                    switch(perimCell.direction) {
                        case NORTH: {
                            SDL_Rect wallRect = {x, y, pixelSize, 2};
                            SDL_RenderFillRect(game->app.renderer, &wallRect);
                            break;
                        }
                        case EAST: {
                            SDL_Rect wallRect = {x + pixelSize - 2, y, 2, pixelSize};
                            SDL_RenderFillRect(game->app.renderer, &wallRect);
                            break;
                        }
                        case SOUTH: {
                            SDL_Rect wallRect = {x, y + pixelSize - 2, pixelSize, 2};
                            SDL_RenderFillRect(game->app.renderer, &wallRect);
                            break;
                        }
                        case WEST: {
                            SDL_Rect wallRect = {x, y, 2, pixelSize};
                            SDL_RenderFillRect(game->app.renderer, &wallRect);
                            break;
                        }
                    }
                }
            }
        }
    }

}