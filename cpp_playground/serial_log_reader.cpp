#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// バイトデータを格納する構造体
struct ByteData {
    uint8_t value;
    std::string color;
    std::string memo;
};

// 色とメモを判定する関数
ByteData parseByteData(uint8_t byte) {
    ByteData data;
    data.value = byte;
    
    // 値に応じて色とメモを設定
    switch (byte) {
        case 0x05:
            data.color = "青";
            data.memo = "ヘッダー1";
            break;
        case 0x01:
            data.color = "緑";
            data.memo = "ヘッダー2";
            break;
        // 必要に応じて他のケースを追加
        default:
            data.color = "白";
            data.memo = "データ";
    }
    return data;
}

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
        // "0501090[126]"のパターンを含む行を検索
        size_t pattern_pos = line.find("0501090");
        if (pattern_pos != std::string::npos && 
            line.length() > pattern_pos + 7 &&
            (line[pattern_pos + 7] == '1' || line[pattern_pos + 7] == '2' || line[pattern_pos + 7] == '6')) {
            
            // パターンから始まる部分を抽出
            std::string extracted = line.substr(pattern_pos);
            
            // バイトデータを構造体の配列に変換
            std::vector<ByteData> byteDataArray;
            for (size_t i = 0; i < extracted.length(); i += 2) {
                if (i + 1 >= extracted.length()) break;
                std::string byteStr = extracted.substr(i, 2);
                uint8_t byte = std::stoi(byteStr, nullptr, 16);
                byteDataArray.push_back(parseByteData(byte));
            }

            // 構造体の内容を表示
            std::cout << line_count << ": ";
            for (const ByteData& data : byteDataArray) {
                printf("値:%02X 色:%s メモ:%s | ", 
                       data.value, 
                       data.color.c_str(), 
                       data.memo.c_str());
            }
            std::cout << std::endl;

            line_count++;
            if (line_count > 1000) break;
        }
    }

    // ファイルを閉じる
    file.close();
    return 0;
} 