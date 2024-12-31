// Special thanks to:
// https://www.noser.com/techblog/first-steps-with-an-usb-hid-report/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

// バイトデータを格納する構造体
struct ByteData
{
    // 上で定義したbytesを要素に入れる
    std::vector<uint8_t> bytes;
    uint32_t color;
    std::string memo;
    uint8_t bSize;
    char bType;
};

ByteData initNewLine(std::vector<uint8_t> bytes)
{
    ByteData data;
    data.bytes = bytes;
    data.color = 0x555555;
    data.memo = "hoge";
    data.bSize = 0;
    data.bType = 'u';
    return data;
}

std::map<int, std::string> collectionMap = {
    {0x00, "Collection(Physical)"},
    {0x01, "Collection(Application)"}};

std::map<int, std::string> coloringMap = {
    {0x05, "UsagePage"},
    {0x09, "Usage"},
    {0x15, "LogicalMinimum"},
    {0x19, "UsageMinimum"},
    {0x29, "UsageMaximum"},
    {0x25, "LogicalMaximum"},
    {0x75, "ReportSize"},
    {0x81, "Input?"},
    {0x85, "ReportID"},
    {0x95, "ReportCount"},
    {0xA1, "Collection"},
    {0xC0, "End Collection"}};

std::map<int, uint32_t> coloringMapColor = {
    {0x05, 0x6666FF},
    {0x09, 0x9999FF},
    {0x15, 0xFFFF22},
    {0x19, 0xCCDDDD},
    {0x29, 0xCCDDDD},
    {0x25, 0xFFFF22},
    {0x75, 0x00FF22},
    {0x81, 0xFF6622},
    {0x85, 0x00FF22},
    {0x95, 0x00FF22},
    {0xA1, 0xCC22CC},
    {0xC0, 0xCC22CC}};

ByteData annotateItem(ByteData data)
{
    uint8_t bSize = 0;
    switch (data.bytes[0] & 0b00000011)
    {
    case 0b00:
        bSize = 1;
        break;
    case 0b01:
        bSize = 2;
        break;
    case 0b10:
        bSize = 3;
        break;
    case 0b11:
        bSize = 4;
        break;
    }

    data.bSize = bSize;

    char bType;
    switch ((data.bytes[0] >> 2) & 0b00000011)
    {
    case 0b00:
        bType = 'M';
        break;
    case 0b01:
        bType = 'G';
        break;
    case 0b10:
        bType = 'L';
        break;
    case 0b11:
        bType = 'S';
        break;
    }

    data.bType = bType;

    uint8_t bTag = data.bytes[0] >> 4;

    if (bTag == 0b0000)
    {
        if (bType == 'G')
        {
            data.memo = "UsagePage";
            data.color = 0x5588FF;
        }
        else if (bType == 'L')
        {
            data.memo = "Usage";
            data.color = 0x5588FF;
        }
    }
    else if (bTag == 0b0001)
    {
        if (bType == 'G')
        {
            data.memo = "LogicalMinimum";
            data.color = 0x00FF00;
        }
        else if (bType == 'L')
        {
            data.memo = "UsageMinimum";
            data.color = 0x22DDDD;
        }
    }
    else if (bTag == 0b0010)
    {
        if (bType == 'G')
        {
            data.memo = "LogicalMaximum";
            data.color = 0x00FF00;
        }
        else if (bType == 'L')
        {
            data.memo = "UsageMaximum";
            data.color = 0x22DDDD;
        }
    }
    else if (bTag == 0b0011)
    {
        if (bType == 'G')
        {
            data.memo = "PhysicalMinimum";
            data.color = 0x00CC55;
        }
        else if (bType == 'L')
        {
            data.memo = "DesignatorIndex";
            data.color = 0xDD0044;
        }
    }
    else if (bTag == 0b0100)
    {
        if (bType == 'G')
        {
            data.memo = "PhysicalMaximum";
            data.color = 0x00CC55;
        }
        else if (bType == 'L')
        {
            data.memo = "DesignatorMinimum";
            data.color = 0x00CC55;
        }
    }
    else if (bTag == 0b0101)
    {
        if (bType == 'G')
        {
            data.memo = "UnitExponent";
            data.color = 0xDDCC55;
        }
        else if (bType == 'L')
        {
            data.memo = "DesignatorMaximum";
            data.color = 0xDDCC55;
        }
    }
    else if (bTag == 0b0110)
    {
        if (bType == 'G')
        {
            data.memo = "Unit";
            data.color = 0x00CC55;
        }
        else if (bType == 'L')
        {
            data.memo = "StringIndex";
            data.color = 0xDD0044;        
        }
    }
    else if (bTag == 0b0111)
    {
        if (bType == 'G')
        {
            data.memo = "ReportSize";
            data.color = 0xFFCC55;
        }
        else if (bType == 'L')
        {
            data.memo = "Delimiter";
            data.color = 0x00CC55;
        }
    }
    else if (bTag == 0b1000)
    {
        if (bType == 'M')
        {
            data.memo = "Input";
            data.color = 0xFF00CC;
        }
        else if (bType == 'G')
        {
            data.memo = "ReportID";
            data.color = 0xFFCC55;
        }
        else if (bType == 'L')
        {
            data.memo = "Unknown";
            data.color = 0x444444;
        }
    }
    else if (bTag == 0b1001)
    {
        if (bType == 'M')
        {
            data.memo = "Output";
            data.color = 0xCCFFCC;
        }
        else if (bType == 'G')
        {
            data.memo = "ReportCount";
            data.color = 0xFFCC55;
        }
        else if (bType == 'L')
        {
            data.memo = "Unknown";
            data.color = 0x444444;
        }
    }
    else if (bTag == 0b1010)
    {
        if (bType == 'M')
        {
            data.memo = "Collection";
            data.color = 0xDD3355;
        }
        else if (bType == 'G')
        {
            data.memo = "Push";
            data.color = 0xDD7777;
        }
        else if (bType == 'L')
        {
            data.memo = "Unknown";
            data.color = 0x444444;
        }
    }
    else if (bTag == 0b1011)
    {
        if (bType == 'M')
        {
            data.memo = "Feature";
            data.color = 0x00CC55;
        }
        else if (bType == 'G')
        {
            data.memo = "Pop";
            data.color = 0xDD7777;
        }
        else if (bType == 'L')
        {
            data.memo = "Unknown";
            data.color = 0x444444;
        }
    }
    else if (bTag == 0b1100)
    {
        if (bType == 'M')
        {
            data.memo = "End Collection";
            data.color = 0xCC22CC;
        }
        else if (bType == 'L')
        {
            data.memo = "Unknown";
            data.color = 0x444444;
        }
    }
    else if (bTag == 0b1101)
    {
        data.memo = "Unknown";
        data.color = 0x444444;
    }
    else if (bTag == 0b1110)
    {
        data.memo = "Unknown";
        data.color = 0x444444;
    }
    else if (bTag == 0b1111)
    {
        data.memo = "Unknown";
        data.color = 0x444444;
    }
    return data;
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
            // if (true)
            // {

            std::string extracted = line.substr(pattern_pos);

            bool line2pass = false;
            bool outerCollection = false;

            std::vector<ByteData> byteDataArray;
            for (size_t i = 0; i + 1 < extracted.length();)
            {
                // 2文字（16進数1バイト分）を切り出し
                std::string byteStr = extracted.substr(i, 2);

                // 現在のバイトを取得
                uint8_t currentByte = std::stoi(byteStr, nullptr, 16);

                // サイズを計算（下位2ビットで判断）
                int byteCount = 1; // 現在のバイトを含む
                switch (currentByte & 0b00000011)
                {
                case 0b00:
                    byteCount = 1;
                    break; // 追加データなし
                case 0b01:
                    byteCount = 2;
                    break; // 1バイト追加
                case 0b10:
                    byteCount = 3;
                    break; // 2バイト追加
                case 0b11:
                    byteCount = 4;
                    break; // 3バイト追加
                }

                // バイトデータを収集
                std::vector<uint8_t> adder;
                for (int j = 0; j < byteCount && (i + j * 2) < extracted.length(); j++)
                {
                    std::string nextByteStr = extracted.substr(i + j * 2, 2);
                    adder.push_back(std::stoi(nextByteStr, nullptr, 16));
                }

                // 次の位置へ移動（16進数文字列なので2倍）
                i += byteCount * 2;

                byteDataArray.push_back(initNewLine(adder));
            }

            for (size_t i = 0; i < byteDataArray.size(); i++)
            {
                byteDataArray[i] = annotateItem(byteDataArray[i]);
            }

            // 構造体の内容を表示
            std::cout << "\nline: " << line_count << std::endl;
            for (size_t i = 0; i < byteDataArray.size(); i++)
            {
                const ByteData &data = byteDataArray[i];

                // if (i % 2 == 0)
                if (true)
                {
                    if (i < 10)
                        std::cout << " ";

                    std::cout << i << ": ";

                    // 最初のバイトを表示
                    printf("\033[38;2;%d;%d;%dm%02X",
                           (data.color >> 16) & 0xFF,
                           (data.color >> 8) & 0xFF,
                           data.color & 0xFF,
                           data.bytes[0]);

                    // 2バイト目以降があれば表示
                    for (size_t j = 1; j < data.bytes.size(); j++)
                    {
                        printf(" %02X", data.bytes[j]);
                    }

                    printf("\033[0m ");
                }
                else
                {
                    // 最初のバイトを表示
                    printf("\033[38;2;%d;%d;%dm%02X",
                           (data.color >> 16) & 0xFF,
                           (data.color >> 8) & 0xFF,
                           data.color & 0xFF,
                           data.bytes[0]);

                    // 2バイト目以降があれば表示
                    for (size_t j = 1; j < data.bytes.size(); j++)
                    {
                        printf("%02X", data.bytes[j]);
                    }

                    printf("  %s\033[0m\n", data.memo.c_str());
                }
                printf(data.memo.c_str());
                printf("\n");

                if (data.bytes[0] == 0xC0)
                {
                    break;
                }
            }
            break;

            line_count++;
            if (line_count > 1000)
                break;
        }
    }

    // ファイルを閉じる
    file.close();
    return 0;
}