#include "Man.h"

// Gets the list of all executable commands with manpages (Sec. 1)
std::vector<std::string> Man::getAllCommands() {
    int buffer_size = 256;
    char stdout_buffer[buffer_size];
    std::vector<std::string> command_vector;
    std::string command = "apropos -s 1 ''";
    std::string command_result = "";
    FILE * pipe = popen(command.c_str(), "r");

    // Check for pipe health
    if (!pipe) {
        const std::string error_msg = "Failed to open stdout pipe()";
        _log.error(CLASS_NAME, error_msg);
        throw std::runtime_error(error_msg);
    }

    // Run the command and get stdout
    try {
        while(fgets(stdout_buffer, sizeof stdout_buffer, pipe) != NULL) {
            command_result += stdout_buffer;
        }
    } catch (...) {
        const std::string error_msg = "Issue reading from stdout pipe()";
        pclose(pipe);
        _log.error(CLASS_NAME, error_msg);
        throw std::runtime_error(error_msg);
    }

    // Parse command lines out of stdout
    if (!command_result.empty()) {
        std::stringstream line_stream(command_result);
        std::string line_delimited;
        
        // Parse each line
        while (std::getline(line_stream, line_delimited, '\n')) {
            std::stringstream command_stream(line_delimited);
            std::string command_delimited;

            // Parse each command from the line
            std::getline(command_stream, command_delimited, ' ');
            command_vector.push_back(command_delimited); 
        }
    } else {
        const std::string error_msg = "Failed to recieve `man` command output";
        _log.error(CLASS_NAME, error_msg);
        throw std::ios_base::failure(error_msg);
    }

    pclose(pipe);
    _log.normal(CLASS_NAME, "Retrieved " + std::to_string(command_vector.size()) + " commands from system.");
    
    return command_vector;
}


// Gets manpage output for a command
std::string Man::getCommandMan(const std::string& command) {
    int buffer_size = 1024;
    char stdout_buffer[buffer_size];
    std::vector<std::string> command_vector;
    std::string man_command = "info " + command + " 2>/dev/null | col -b";
    std::string command_result = "";
    FILE * pipe = popen(man_command.c_str(), "r");

    // Check for pipe health
    if (!pipe) {
        const std::string error_msg = "Failed to open stdout pipe()";
        _log.error(CLASS_NAME, error_msg);
        throw std::runtime_error(error_msg);
    }

    // Run the command and get stdout
    try {
        int counter = 0;
        while(fgets(stdout_buffer, sizeof stdout_buffer, pipe) != NULL) {
            command_result += stdout_buffer;

            if (counter++ >= _max_chunks) {
                break;
            }
        }
    } catch (...) {
        const std::string error_msg = "Issue reading from stdout pipe()";
        pclose(pipe);
        _log.error(CLASS_NAME, error_msg);
        throw std::runtime_error(error_msg);
    }

    pclose(pipe);

    return command_result;
}


std::vector<Chunk> Man::getCommandChunks(const std::string& command, const std::string& man_contents) {
    std::vector<Chunk> man_chunks;
    int chunk_size = _config._config["embedding"]["chunk_size"].get<int>();
    std::string contents = man_contents;

    Pipeline().run(contents);

    for (size_t i = 0; i < contents.size(); i += chunk_size) {
        std::string chunk_content = contents.substr(i, chunk_size);
        pgvector::Vector embedding;

        man_chunks.push_back(Chunk(command, chunk_content, embedding));
    }

    return man_chunks;
}