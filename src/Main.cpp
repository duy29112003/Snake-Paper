#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <SDL.h>
#include <SDL_image.h>

using namespace std;

//**************************************************************
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 720;
const string WINDOW_TITLE = "Snake Paper";

const int FPS = 60;
const int DELAY_TIME = 1000 / FPS;

Uint32 frameStart;
Uint32 frameTime;

const int N = 18, M = 22;
const int BLOCK_SIZE = 25;

int boardX = 80, boardY = 100;
int boardW = 630 - 80, boardH = 550 - 100;

map<string, vector<int> > dir;
vector<int> food;


SDL_Texture * bgImg = NULL;
SDL_Texture * inventory = NULL;
SDL_Texture * foodImg = NULL;

SDL_Texture * head_left = NULL;
SDL_Texture * head_right = NULL;
SDL_Texture * head_up = NULL;
SDL_Texture * head_down = NULL;

SDL_Texture * tail_left = NULL;
SDL_Texture * tail_right = NULL;
SDL_Texture * tail_up = NULL;
SDL_Texture * tail_down = NULL;

SDL_Texture * body_horizontal = NULL;
SDL_Texture * body_vertical = NULL;

SDL_Texture * north_west = NULL;
SDL_Texture * north_east = NULL;
SDL_Texture * south_west = NULL;
SDL_Texture * south_east = NULL;

SDL_Texture * startImg = NULL;
SDL_Texture * replayImg = NULL;
bool start = false;

SDL_Rect filled_rect;


void logSDLError(std::ostream& os, 
                 const std::string &msg, bool fatal = false);
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);
void quitSDL(SDL_Window* window, SDL_Renderer* renderer);
void waitUntilKeyPressed();

SDL_Texture* loadTexture(SDL_Renderer* renderer, string path);
//**************************************************************

struct Point {
    int i, j;
    Point(int _i, int _j) {i = _i; j = _j; };
};

Point getCoordinate(int i, int j) { return Point(i * BLOCK_SIZE + boardY, j * BLOCK_SIZE + boardX); }
SDL_Rect getTile(Point point) { SDL_Rect tile = {point.j, point.i, BLOCK_SIZE, BLOCK_SIZE}; return tile;}

class Snake {
private:

    vector<SDL_Texture*> body;

    vector<Point> queue;
    vector<int> snake_dir;
    int board[N][M];

    bool die;

    int TIME_MOVE = 5;
    int counter;
    int food_nums;


public:  
    Snake() { reset(); }
    bool isDie() { return die; };
    void setDie(bool flag) { die = flag; };

    void reset() {
        queue.clear();
        queue.push_back(Point(4,4));
        queue.push_back(Point(4,5));
        queue.push_back(Point(4,6));

        for (int i = 0; i < N; i++) for (int j = 0; j < M; j++) board[i][j] = 0;
        board[4][4] = board[4][5] = board[4][6] = 1;

        snake_dir = dir["right"];
        counter = TIME_MOVE;
        die = false;
        food_nums = 0;

        food = {N/2, M/2};
    }

    int getFoodNums() { return food_nums; };

    void changeDir(vector<int> new_dir) { 
        if (snake_dir[0] + new_dir[0] == 0 and snake_dir[1] + new_dir[1] == 0) return; 
        snake_dir = new_dir;
    }

    bool collision(int i, int j) { 
        if ( i < 0 or j < 0 or i >= N or j >= M) return true; 
        for (int t = 0; t < queue.size(); t++) {
            if (i == queue[t].i and j == queue[t].j ) return true; 
        }
        return false;
    };
    
    void randomFood() {
        vector<Point> coors;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M;j ++) {
                if (board[i][j] == false) coors.push_back(Point(i, j));
            }
        }


        srand(time(NULL)); 
	    int res = rand() % (coors.size() - 0) + 0;

        food[0] = coors[res].i;
        food[1] = coors[res].j;
    }

    bool eatFood(int i, int j) { return i == food[0] and j == food[1]; }

    void move() {
        Point head = queue.back();

        Point newHead = Point(head.i + snake_dir[0], head.j + snake_dir[1]);

        if (!collision(newHead.i, newHead.j)) {

            queue.push_back(newHead);
            board[newHead.i][newHead.j] = true;
            if (!eatFood(newHead.i, newHead.j)) {
                board[queue[0].i][queue[0].j] = false;
                queue.erase(queue.begin());
            } else {
                food_nums ++;
                randomFood();
            }
        } else {
            die = true;
        }
    }

    void update() {
        // if (die) return;

        counter --;

        if (counter == 0) {
            counter = TIME_MOVE;
            move();
        }

        if (die) {
            reset();
        }
    }


    void draw(SDL_Renderer * renderer) {

        // Vẽ đầu
        Point head = queue.back();
        SDL_Rect head_rect = getTile(getCoordinate(head.i, head.j));

        if (snake_dir == dir["left"]) SDL_RenderCopy(renderer, head_left, NULL, &head_rect);
        if (snake_dir == dir["right"]) SDL_RenderCopy(renderer, head_right, NULL, &head_rect);
        if (snake_dir == dir["up"]) SDL_RenderCopy(renderer, head_up, NULL, &head_rect);
        if (snake_dir == dir["down"]) SDL_RenderCopy(renderer, head_down, NULL, &head_rect);


        // Vẽ đuôi
        Point tail = queue[0];
        vector<int> tail_dir = {queue[0].i - queue[1].i, queue[0].j - queue[1].j};
        SDL_Rect tail_rect = getTile(getCoordinate(tail.i, tail.j));
        
        if (tail_dir == dir["left"]) SDL_RenderCopy(renderer, tail_left, NULL, &tail_rect);
        if (tail_dir == dir["right"]) SDL_RenderCopy(renderer, tail_right, NULL, &tail_rect);
        if (tail_dir == dir["up"]) SDL_RenderCopy(renderer, tail_up, NULL, &tail_rect);
        if (tail_dir == dir["down"]) SDL_RenderCopy(renderer, tail_down, NULL, &tail_rect);


        // Vẽ thân
        for (int i = 1; i < queue.size() - 1; i++) {
            vector<int> fi = {queue[i-1].i - queue[i].i, queue[i-1].j - queue[i].j};
            vector<int> se = {queue[i+1].i - queue[i].i, queue[i+1].j - queue[i].j};

            SDL_Rect tile_rect = getTile(getCoordinate(queue[i].i, queue[i].j));

            if (fi == dir["left"]) {
                if (se == dir["right"]) SDL_RenderCopy(renderer, body_horizontal, NULL, &tile_rect);
                if (se == dir["up"]) SDL_RenderCopy(renderer, south_east, NULL, &tile_rect);
                if (se == dir["down"]) SDL_RenderCopy(renderer, north_east, NULL, &tile_rect);
            }
            if (fi == dir["right"]) {
                if (se == dir["left"]) SDL_RenderCopy(renderer, body_horizontal, NULL, &tile_rect);
                if (se == dir["up"]) SDL_RenderCopy(renderer, south_west, NULL, &tile_rect);
                if (se == dir["down"]) SDL_RenderCopy(renderer, north_west, NULL, &tile_rect);
            }
            if (fi == dir["up"]) {
                if (se == dir["right"]) SDL_RenderCopy(renderer, south_west, NULL, &tile_rect);
                if (se == dir["left"]) SDL_RenderCopy(renderer, south_east, NULL, &tile_rect);
                if (se == dir["down"]) SDL_RenderCopy(renderer, body_vertical, NULL, &tile_rect);
            }
            if (fi == dir["down"]) {
                if (se == dir["right"]) SDL_RenderCopy(renderer, north_west, NULL, &tile_rect);
                if (se == dir["up"]) SDL_RenderCopy(renderer, body_vertical, NULL, &tile_rect);
                if (se == dir["left"]) SDL_RenderCopy(renderer, north_east, NULL, &tile_rect);
            }
        }
    }
    

};


void refreshScreen(SDL_Window* window, SDL_Renderer* renderer, Snake * snake);

int main(int argc, char* argv[]) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer);

    // Your code here
    SDL_Event e;

   
    dir["left"] = {0, -1};
    dir["right"] = {0, 1};
    dir["up"] = {-1, 0};
    dir["down"] = {1, 0};


    bgImg = loadTexture(renderer, "res/img/bg.png");
    foodImg = loadTexture(renderer, "res/img/food.png");
    inventory = loadTexture(renderer, "res/img/inventory.png");

    head_left = loadTexture(renderer, "res/img/head_left.png");
    head_right = loadTexture(renderer, "res/img/head_right.png");
    head_up = loadTexture(renderer, "res/img/head_up.png");
    head_down = loadTexture(renderer, "res/img/head_down.png");

    tail_left = loadTexture(renderer, "res/img/tail_left.png");
    tail_right = loadTexture(renderer, "res/img/tail_right.png");
    tail_up = loadTexture(renderer, "res/img/tail_up.png");
    tail_down = loadTexture(renderer, "res/img/tail_down.png");

    body_horizontal = loadTexture(renderer, "res/img/body_horizontal.png");
    body_vertical = loadTexture(renderer, "res/img/body_vertical.png");

    north_west = loadTexture(renderer, "res/img/angle_nw.png");
    north_east = loadTexture(renderer, "res/img/angle_ne.png");
    south_west = loadTexture(renderer, "res/img/angle_sw.png");
    south_east = loadTexture(renderer, "res/img/angle_se.png");

    startImg = loadTexture(renderer, "res/img/start.png");
    replayImg = loadTexture(renderer, "res/img/replay.png");

    Snake * snake = new Snake();
    snake->setDie(true);

    food.push_back(N / 2);
    food.push_back(M / 2);


    vector<vector<int> > board(N, vector<int> (M));

    filled_rect.x = boardX - 2;
    filled_rect.y = boardY - 2;
    filled_rect.w = boardW + 4;
    filled_rect.h = boardH + 4;

    // Bước nhảy mỗi khi dịch chuyển
    int step = 4;
    // Xoá toàn bộ màn hình và vẽ
    refreshScreen(window, renderer, snake);

    bool quit = false;

    while (!quit) {
        // Đợi 10 mili giây
        frameStart = SDL_GetTicks();

        while( SDL_PollEvent( &e ) != 0 ) {
            //User requests quit
            if (e.type == SDL_QUIT ) quit = true;
            
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT) snake->changeDir(dir["left"]);
                if (e.key.keysym.sym == SDLK_RIGHT) snake->changeDir(dir["right"]);
                if (e.key.keysym.sym == SDLK_DOWN) snake->changeDir(dir["down"]);
                if (e.key.keysym.sym == SDLK_UP) snake->changeDir(dir["up"]);
            }
        }
            
        // Xoá toàn bộ màn hình và vẽ lại
        refreshScreen(window, renderer, snake);

        frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < DELAY_TIME) SDL_Delay(DELAY_TIME - frameTime);
		
    }
    
    quitSDL(window, renderer);
    return 0;
}

// Xoá và vẽ lại toàn bộ màn hình với 1 hình chữ nhật
void refreshScreen(SDL_Window* window, SDL_Renderer* renderer, Snake * snake) {
    // Đặt màu vẽ thành xanh lam (blue), xoá màn hình về màu xanh lam.
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);   // blue
    SDL_RenderClear(renderer);


    // Vẽ background
    SDL_RenderCopy(renderer, bgImg, NULL, NULL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);   // white
    SDL_RenderDrawRect(renderer, &filled_rect);

    // Vẽ kho chứa táo
    SDL_Rect inventory_rect = {670, 0, 300, 600};
    SDL_RenderCopy(renderer, inventory, NULL, &inventory_rect);
    int num = snake->getFoodNums();
    int count = num;
    int max_width = 7;

    for (int i = 0; i < num / max_width + 1 ; i++) {
        for (int j = 0; j < max_width; j++) {
            if (count > 0) {
                count --;
                SDL_Rect rect = {720 + j * 30, 80 + i * 30, BLOCK_SIZE, BLOCK_SIZE};
                SDL_RenderCopy(renderer, foodImg, NULL, &rect);
            }
        }
    }


    // Vẽ rắn
    snake->update();
    snake->draw(renderer);

    // Vẽ thức ăn
    if (!snake->isDie()) {
        SDL_Rect food_rect = getTile(getCoordinate(food[0], food[1]));
        SDL_RenderCopy(renderer, foodImg, NULL, &food_rect);
    }


    // Vẽ nút bắt đầu và chơi lại
    // if (snake->isDie()) {
    //     if (start == false) {

    //         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);   // white

    //         SDL_Rect rect = {430, (720 - 42)/2, 100, 42};

    //         SDL_RenderCopy(renderer, startImg, NULL, &rect);
    //     } else {

    //     }
    // }

    // Dùng lệnh hiển thị (đưa) hình đã vẽ ra mành hình
   //Khi thông thường chạy với môi trường bình thường ở nhà
    SDL_RenderPresent(renderer);
   //Khi chạy ở máy thực hành WinXP ở trường (máy ảo)
   //SDL_UpdateWindowSurface(window);
}


//*****************************************************
// Các hàm chung về khởi tạo và huỷ SDL
void logSDLError(std::ostream& os, const std::string &msg, bool fatal) {
    os << msg << " Error: " << SDL_GetError() << std::endl;
    if (fatal) {
        SDL_Quit();
        exit(1);
    }
}

void initSDL(SDL_Window* &window, SDL_Renderer* &renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        logSDLError(std::cout, "SDL_Init", true);

    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    //window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED, 
    //   SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == nullptr) logSDLError(std::cout, "CreateWindow", true);


    //Khi thông thường chạy với môi trường bình thường ở nhà
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | 
                                              SDL_RENDERER_PRESENTVSYNC);
    //Khi chạy ở máy thực hành WinXP ở trường (máy ảo)
    //renderer = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));
    if (renderer == nullptr) logSDLError(std::cout, "CreateRenderer", true);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void quitSDL(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void waitUntilKeyPressed() {
    SDL_Event e;
    while (true) {
        if ( SDL_WaitEvent(&e) != 0 &&
             (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) )
            return;
        SDL_Delay(100);
    }
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, string path) {
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL ) {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
        if( newTexture == NULL ) {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

// void close() {
//     //Free loaded image
//     SDL_DestroyTexture(bgImg);
//     bgImg = NULL;

//     //Quit SDL subsystems
//     IMG_Quit();
// }

//**************************************************************