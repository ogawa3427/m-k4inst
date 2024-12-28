#include <iostream>
#include <fstream>
#include <string>

int main() {
    // ファイルを開く
    std::ifstream file("data.txt");
    std::string line;

    // ファイルが正常に開けたか確認
    if (!file.is_open()) {       
        std::cerr << "ファイルを開けませんでした。" << std::endl;
        return 1;
    }

    int line_count = 0;

    // 1行ずつ読み込んで表示
    while (std::getline(file, line)) {
        // "hexString: "で始まる行を見つけた場合
        if (line.find("hexString: ") != std::string::npos) {
            // "hexString: "の後の部分（16進数の文字列）だけを抽出
            std::string hexValue = line.substr(11);
            line_count++;
            if (hexValue.find("FE") != 0) {
                continue;
            }


            if (line_count > 1000) {
                break;
            }

            std::cout << line_count << ": " << hexValue << std::endl;
            
        }
    }

    // ファイルを閉じる
    file.close();
    return 0;
} 