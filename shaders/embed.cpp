#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool isValidCppIdentifier(const std::string& s) {
    if (s.empty()) return false;
    if (!(std::isalpha((unsigned char)s[0]) || s[0] == '_')) return false;
    for (char c : s) {
        if (!(std::isalnum((unsigned char)c) || c == '_')) return false;
    }
    return true;
}

static std::vector<std::uint8_t> readAllBytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) throw std::runtime_error("failed to open input: " + path);

    std::streamsize size = file.tellg();
    if (size <= 0) throw std::runtime_error("input is empty/invalid: " + path);

    std::vector<std::uint8_t> data((size_t)size);
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        throw std::runtime_error("failed to read input: " + path);
    }
    return data;
}

static std::vector<std::uint32_t> bytesToWordsLE(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() % 4 != 0) throw std::runtime_error("SPIR-V size is not multiple of 4 bytes.");
    std::vector<std::uint32_t> words(bytes.size() / 4);

    for (size_t i = 0; i < words.size(); ++i) {
        size_t b = i * 4;
        // SPIR-V es little-endian
        words[i] = (std::uint32_t)bytes[b + 0]
                | ((std::uint32_t)bytes[b + 1] << 8)
                | ((std::uint32_t)bytes[b + 2] << 16)
                | ((std::uint32_t)bytes[b + 3] << 24);
    }
    return words;
}

static void writeHeader(const std::string& outPath,
                        const std::string& symbol,
                        const std::string& ns,
                        int columns,
                        const std::vector<std::uint32_t>& words) {
    std::ofstream out(outPath, std::ios::binary);
    if (!out.is_open()) throw std::runtime_error("failed to open output: " + outPath);

    out << "#pragma once\n"
           "#include <cstdint>\n"
           "#include <span>\n\n";

    auto openNamespace = [&]() {
        if (ns.empty()) return;
        // ns: "ThING::shaders" -> namespaces anidados
        size_t start = 0;
        while (start < ns.size()) {
            size_t pos = ns.find("::", start);
            std::string part = (pos == std::string::npos) ? ns.substr(start) : ns.substr(start, pos - start);
            out << "namespace " << part << " {\n";
            if (pos == std::string::npos) break;
            start = pos + 2;
        }
        out << "\n";
    };

    auto closeNamespace = [&]() {
        if (ns.empty()) return;
        // cuenta cuántos niveles hay
        int levels = 1;
        for (size_t i = 0; i + 1 < ns.size(); ++i) {
            if (ns[i] == ':' && ns[i + 1] == ':') levels++;
        }
        out << "\n";
        for (int i = 0; i < levels; ++i) out << "} // namespace\n";
    };

    openNamespace();

    out << "inline constexpr std::uint32_t " << symbol << "_words[] = {\n";

    out.setf(std::ios::hex, std::ios::basefield);
    out.setf(std::ios::showbase);
    out.setf(std::ios::uppercase);

    for (size_t i = 0; i < words.size(); ++i) {
        out << "  0x";
        // imprimir 8 hex digits
        std::uint32_t w = words[i];
        static const char* hex = "0123456789ABCDEF";
        for (int shift = 28; shift >= 0; shift -= 4) {
            out << hex[(w >> shift) & 0xF];
        }
        out << ",";
        if (columns > 0 && ((int)(i + 1) % columns == 0)) out << "\n";
        else out << " ";
    }

    out << "\n};\n\n";
    out << "inline constexpr std::span<const std::uint32_t> " << symbol
        << " = " << symbol << "_words;\n";

    closeNamespace();
}

int main(int argc, char** argv) {
    try {
        if (argc < 4) {
            std::cerr << "Usage:\n  " << argv[0]
                      << " <input.spv> <output.h> <symbol> [namespace] [columns]\n\n"
                         "Example:\n  " << argv[0]
                      << " build/shaders/basicVert.spv build/generated/basicVert_spv.h basicVertSpv ThING::shaders 8\n";
            return 2;
        }

        std::string input = argv[1];
        std::string output = argv[2];
        std::string symbol = argv[3];
        std::string ns = (argc >= 5) ? argv[4] : "";
        int columns = (argc >= 6) ? std::max(1, std::atoi(argv[5])) : 8;

        if (!isValidCppIdentifier(symbol)) {
            std::cerr << "error: invalid C++ identifier for symbol: " << symbol << "\n";
            return 2;
        }

        auto bytes = readAllBytes(input);
        auto words = bytesToWordsLE(bytes);
        writeHeader(output, symbol, ns, columns, words);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "embed_spirv error: " << e.what() << "\n";
        return 1;
    }
}
