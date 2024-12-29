#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// バイトデータを格納する構造体
struct ByteData
{
    uint8_t value;
    uint32_t color;
    std::string memo;
};

// 色とメモを判定する関数
ByteData parseByteData(uint8_t byte, std::string memo, uint32_t color)

{
    ByteData data;
    data.value = byte;
    data.color = color;
    data.memo = memo;
    return data;
}

std::vector<ByteData> doubleParse(uint8_t byteA, uint8_t byteB, std::string memo, uint32_t color)
{
    std::vector<ByteData> byteDataArray;
    byteDataArray.push_back(parseByteData(byteA, memo, color));
    byteDataArray.push_back(parseByteData(byteB, memo, color));
    return byteDataArray;
}

int main()
{
    // ファイルを開く
    std::ifstream file("data.txt");
    std::string line;

    // ファイルが正常に開けたか確認
    if (!file.is_open())
    {
        std::cerr << "ファイルを開けませんでした。" << std::endl;
        return 1;
    }

    int line_count = 0;

    // 1行ずつ読み込んで表示
    while (std::getline(file, line))
    {
        // "0501090[126]"のパターンを含む行を検索
        size_t pattern_pos = line.find("0501090");
        if (pattern_pos != std::string::npos &&
            line.length() > pattern_pos + 7 &&
            (line[pattern_pos + 7] == '1' || line[pattern_pos + 7] == '2' || line[pattern_pos + 7] == '6'))
        {

            std::string extracted = line.substr(pattern_pos);


            // 2バイト単位でデータを処理
            std::vector<ByteData> byteDataArray;
            for (size_t i = 0; i + 3 < extracted.length(); i += 4)
            {
                std::string wordStr = extracted.substr(i, 4);
                uint16_t word = std::stoi(wordStr, nullptr, 16);

                uint8_t highByte = (word >> 8) & 0xFF;
                uint8_t lowByte = word & 0xFF;

                std::vector<ByteData> parsedData = doubleParse(highByte, lowByte, "hoge", 0x555555);

                if (i == 0 && highByte == 0x05 && lowByte == 0x01)
                {
                    parsedData = doubleParse(0x05, 0x01, "UsagePage(Generic Desktop)", 0x6666FF);
                }
                else if (i == 1*4 && highByte == 0x09 && (lowByte == 0x02 || lowByte == 0x01 || lowByte == 0x06))
                {
                    if (lowByte == 0x02)
                        parsedData = doubleParse(highByte, lowByte, "Usage(Mouse)", 0x9999FF);
                    else if (lowByte == 0x01)
                        parsedData = doubleParse(highByte, lowByte, "Usage(Pointer)", 0x9999FF);
                    else if (lowByte == 0x06)
                        parsedData = doubleParse(highByte, lowByte, "Usage(Keyboard)", 0x9999FF);
                }
                byteDataArray.insert(byteDataArray.end(), parsedData.begin(), parsedData.end());
            }

            // 構造体の内容を表示
            std::cout << "\nline: " << line_count << std::endl;
            for (size_t i = 0; i < byteDataArray.size(); i++)
            {
                const ByteData &data = byteDataArray[i];

                if (i % 2 == 0)
                {
                    if (i < 10)
                        std::cout << " ";
                   
                    std::cout << i << ": ";

                    printf("\033[38;2;%d;%d;%dm%02X\033[0m ",
                           (data.color >> 16) & 0xFF,
                           (data.color >> 8) & 0xFF,
                           data.color & 0xFF,
                           data.value);
                }
                else
                {
                    printf("\033[38;2;%d;%d;%dm%02X  %s\033[0m\n",
                           (data.color >> 16) & 0xFF,
                           (data.color >> 8) & 0xFF,
                           data.color & 0xFF,
                           data.value,
                           data.memo.c_str());
                }
            }

            line_count++;
            if (line_count > 1000)
                break;
        }
    }

    // ファイルを閉じる
    file.close();
    return 0;
}