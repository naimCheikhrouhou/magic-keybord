#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>

class Block {
public:
    sf::RectangleShape shape;
    sf::Text text;
    char key;
    float speed;
    bool active;

    Block(char k, float spd, sf::Font& font) : key(k), speed(spd), active(true) {
        shape.setSize(sf::Vector2f(80.f, 40.f));
        shape.setFillColor(sf::Color::Blue);
        
        text.setFont(font);
        text.setString(std::string(1, k));
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
    }

    void update(float dt) {
        shape.move(0.f, speed * dt);
        text.setPosition(shape.getPosition() + sf::Vector2f(30.f, 5.f));
    }
};

    class Game {
    private:
        sf::RenderWindow window;
        sf::Font font;
        sf::Music music;
        std::vector<Block> blocks;
        int score;
        int highScore;
        bool gameOver;
        bool replayClicked;
        sf::Clock spawnClock;
        sf::SoundBuffer buffer;
        sf::Sound hitSound;
        std::map<sf::Keyboard::Key, char> validKeys;
        float nextSpawnTime;
        const float beatInterval = 800.f;
        const float targetZone = 500.f;
        sf::RectangleShape replayButton;
        private:
        float baseSpeed = 50.f;        // Vitesse de base
        float difficultyCurve = 0.1f;   // Courbe de difficulté
        float maxSpeed = 250.f;         // Vitesse maximale
        int difficultyThreshold = 50;   // Seuil de score pour l'augmentation
        float horizontalSpacing = 120.f; // Distance minimale entre les blocs
        int maxConcurrentBlocks = 3;     // Nombre max de blocs simultanés
        std::vector<sf::Texture> backgroundTextures;
        sf::Sprite backgroundSprite;
        int currentDifficultyLevel = 0;
        const std::vector<std::string> difficultyBackgrounds = {
            "assets/backgrounds/bg_easy.jpg",    // Niveau 0
            "assets/backgrounds/bg_medium.jpg",  // Niveau 1
            "assets/backgrounds/bg_hard.jpg",    // Niveau 2
            "assets/backgrounds/bg_extreme.jpg"  // Niveau 3
        };


    public:
        Game() : window(sf::VideoMode(800, 600), "Magic Tiles"), score(0), gameOver(false), nextSpawnTime(0) {
            window.setFramerateLimit(60);
            if (!font.loadFromFile("assets/fonts/arial.ttf")) exit(1);
            if (!buffer.loadFromFile("assets/sounds/hit.ogg")) exit(1);
            hitSound.setBuffer(buffer);
            if (!music.openFromFile("assets/sounds/music.ogg")) exit(1);
            music.play();

            loadHighScore();

            validKeys = {
                {sf::Keyboard::A, 'A'}, {sf::Keyboard::B, 'B'}, {sf::Keyboard::C, 'C'},
                {sf::Keyboard::D, 'D'}, {sf::Keyboard::E, 'E'}, {sf::Keyboard::F, 'F'},
                {sf::Keyboard::G, 'G'}, {sf::Keyboard::H, 'H'}, {sf::Keyboard::I, 'I'},
                {sf::Keyboard::J, 'J'}, {sf::Keyboard::K, 'K'}, {sf::Keyboard::L, 'L'},
                {sf::Keyboard::M, 'M'}, {sf::Keyboard::N, 'N'}, {sf::Keyboard::O, 'O'},
                {sf::Keyboard::P, 'P'}, {sf::Keyboard::Q, 'Q'}, {sf::Keyboard::R, 'R'},
                {sf::Keyboard::S, 'S'}, {sf::Keyboard::T, 'T'}, {sf::Keyboard::U, 'U'},
                {sf::Keyboard::V, 'V'}, {sf::Keyboard::W, 'W'}, {sf::Keyboard::X, 'X'},
                {sf::Keyboard::Y, 'Y'}, {sf::Keyboard::Z, 'Z'}
            };

            // Configure le bouton de replay
            replayButton.setSize(sf::Vector2f(200.f, 50.f));
            replayButton.setFillColor(sf::Color::Green);
            replayButton.setPosition(300.f, 350.f);
            // Chargement des textures
    for (const auto& bg : difficultyBackgrounds) {
        sf::Texture tex;
        if (!tex.loadFromFile(bg)) {
            std::cerr << "Failed to load: " << bg << std::endl;
            exit(1);
        }
        tex.setSmooth(true); // Ajouté pour un meilleur rendu
        backgroundTextures.push_back(tex);
    }
    backgroundSprite.setTexture(backgroundTextures[0], true);
        
        }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
void spawnBlock() {
    // Vérifie le nombre de blocs actifs
    if(blocks.size() >= maxConcurrentBlocks) return;

    char randomKey = 'A' + rand() % 26;
    
    // Génère des positions X avec espacement
    float x;
    bool validPosition;
    do {
        validPosition = true;
        x = rand() % (window.getSize().x - 80);
        
        // Vérifie l'espacement avec les autres blocs
        for(const auto& block : blocks) {
            if(std::abs(block.shape.getPosition().x - x) < horizontalSpacing) {
                validPosition = false;
                break;
            }
        }
    } while(!validPosition);

    blocks.emplace_back(randomKey, baseSpeed, font);
    blocks.back().shape.setPosition(x, 0.f);
}

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::KeyPressed && !gameOver) {
                handleKeyPress(event.key.code);
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (gameOver && replayButton.getGlobalBounds().contains(
                    static_cast<float>(event.mouseButton.x), 
                    static_cast<float>(event.mouseButton.y))) {
                    resetGame();
                }
            }
        }
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        if (validKeys.find(key) != validKeys.end()) {
            char pressed = validKeys[key];
            for (auto& block : blocks) {
                if (block.active && block.key == pressed) {
                    if (block.shape.getPosition().y + block.shape.getSize().y >= targetZone) {
                        block.active = false;
                        score += 10;
                        hitSound.play();
                    }
                }
            }
        }
    }

    void update() {
        if (gameOver) return;
    
        float dt = 1.f / 60.f;
    
        // Gestion du spawn des blocs rythmique
        if (music.getPlayingOffset().asMilliseconds() > nextSpawnTime) {
            spawnBlock();
            nextSpawnTime += beatInterval;
        }
    
        // Mise à jour des positions des blocs
        for (auto& block : blocks) {
            block.update(dt);
            
            // Vérification de la chute des blocs
            if (block.shape.getPosition().y > window.getSize().y) {
                gameOver = true;
                if (score > highScore) {
                    highScore = score;
                    saveHighScore();
                }
            }
        }
    
        // --------------------------------------------------
        // SECTION D'AJOUT : PROGRESSION DE DIFFICULTÉ
        // --------------------------------------------------
        if (score > 0 && score % difficultyThreshold == 0) 
        {
            // Augmentation exponentielle de la courbe
            difficultyCurve *= 1.1f;
            
            // Augmentation progressive des paliers
            difficultyThreshold += 50; 
            
            // Bonus de vitesse de base
            baseSpeed += 15.f;
            
            // Feedback sonore d'avertissement
            if(!gameOver) hitSound.play();
            
            // Protection contre la vitesse excessive
            if(baseSpeed > maxSpeed) baseSpeed = maxSpeed;
        }
        // --------------------------------------------------
    
        // Nettoyage des blocs inactifs
        blocks.erase(std::remove_if(blocks.begin(), blocks.end(),
            [](const Block& b) { return !b.active; }), blocks.end());
             // Détermination du niveau de difficulté
    int newLevel = std::min(
        static_cast<int>(baseSpeed - 150.f) / 50, 
        static_cast<int>(difficultyBackgrounds.size() - 1)
    );

    if (newLevel != currentDifficultyLevel) {
        currentDifficultyLevel = newLevel;
        backgroundSprite.setTexture(backgroundTextures[currentDifficultyLevel]);
        
        // Animation de transition
        backgroundSprite.setColor(sf::Color(255, 255, 255, 0));
    }

    // Transition en fondu
    if (backgroundSprite.getColor().a < 255) {
        backgroundSprite.setColor(sf::Color(255, 255, 255, 
            std::min(backgroundSprite.getColor().a + 2, 255)));
    }
    }

    void render() {
        window.clear(sf::Color::Black);
    
    // Draw target zone
    sf::RectangleShape targetLine(sf::Vector2f(window.getSize().x, 2.f));
    targetLine.setPosition(0.f, targetZone);
    targetLine.setFillColor(sf::Color::Red);
    window.draw(targetLine);

    // Draw blocks
    for (auto& block : blocks) {
        if (block.active) {
            window.draw(block.shape);
            window.draw(block.text);
        }
    }

    // --------------------------------------------------
    // AJOUT DE L'EFFET VIGNETTAGE ICI
    // --------------------------------------------------
    sf::RectangleShape vignette(sf::Vector2f(window.getSize()));
    vignette.setFillColor(sf::Color(0, 0, 0, 50)); // Opacité 20%
    window.draw(vignette);
    

    // Draw UI (au-dessus de la vignette pour lisibilité)
    sf::Text scoreText("Score: " + std::to_string(score), font, 30);
    scoreText.setPosition(10.f, 10.f);
    window.draw(scoreText);

    sf::Text highScoreText("High Score: " + std::to_string(highScore), font, 30);
    highScoreText.setPosition(500.f, 10.f);
    window.draw(highScoreText);

        // Game over screen
        if (gameOver) {
            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            
            sf::Text goText("Game Over!", font, 50);
            goText.setPosition(250.f, 200.f);
            goText.setFillColor(sf::Color::White);
            
            sf::Text finalScoreText("Final Score: " + std::to_string(score), font, 40);
            finalScoreText.setPosition(250.f, 260.f);
            
            window.draw(overlay);
            window.draw(goText);
            window.draw(finalScoreText);
            window.draw(replayButton);
            
            sf::Text replayText("Replay", font, 30);
            replayText.setPosition(365.f, 365.f);
            window.draw(replayText);
        }

        window.display();
    }
    void resetGame() {
        score = 0;
        blocks.clear();
        gameOver = false;
        nextSpawnTime = 0;
        spawnClock.restart();
        music.stop();
        music.play();
    }

    void loadHighScore() {
        std::ifstream file("highscore.txt");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
        }
    }

    void saveHighScore() {
        std::ofstream file("highscore.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
};

int main() {
    srand(time(0));
    Game game;
    game.run();
    return 0;
}
