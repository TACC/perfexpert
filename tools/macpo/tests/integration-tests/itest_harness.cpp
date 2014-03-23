
#include <fcntl.h>
#include <rose.h>
#include <sys/wait.h>

#include <fstream>
#include <string>
#include <vector>

#include "generic_defs.h"
#include "itest_harness.h"
#include "minst.h"

bool file_exists(const std::string& filename) {
    return access(filename.c_str(), F_OK) != -1;
}

std::string get_src_directory() {
    return std::string(getenv("srcdir"));
}

std::string get_build_directory() {
    char result[1024];
    ssize_t count = readlink("/proc/self/exe", result, 1024);
    std::string exe_path = std::string(result, (count > 0) ? count : 0);

    size_t found = exe_path.find_last_of("/");
    return exe_path.substr(0, found);
}

std::string get_tests_directory() {
    return get_src_directory() + "/test-files";
}

std::string instrument_and_link(std::string input_file,
        string_list_t* special_args, options_t& options) {
    assert(file_exists(input_file) == true);
    std::string output_file = std::tmpnam(NULL);

    string_list_t args;
    populate_args(args, input_file, output_file, special_args);

    SgProject* project = frontend(args);

    // Call the instrumentation code.
    MINST traversal(options, project);
    traversal.traverseInputFiles(project, preorder);

    // Compile the files down to the binary.
    assert(backend(project) == 0);

    delete project;
    return output_file;
}

bool verify_output(std::string& filename, std::string& binary) {
    std::string program_output = get_process_stderr(binary);
    std::string file_contents = get_file_contents(filename);

    string_list_t prog_line_list, file_line_list;
    split(program_output, '\n', prog_line_list);
    split(file_contents, '\n', file_line_list);

    size_t file_lines = file_line_list.size();
    size_t prog_lines = prog_line_list.size();

    if (file_lines < prog_lines) {
        // We have more expected output than the lines in the file
        // (which include the expected output), report error.
        std::cout << "File contents smaller than program output: " << 
            file_lines << " v/s " << prog_lines << "!" << std::endl;
        return false;
    }

    size_t ctr = 0;
    std::string expected_prefix = "[macpo-integration-test]:";
    for (string_list_t::iterator it = prog_line_list.begin();
            it != prog_line_list.end(); it++) {
        std::string prog_line = *it;
        if (prog_line.compare(0, expected_prefix.size(),
                expected_prefix) == 0) {
            string_list_t prog_field_list;
            split(prog_line, ':', prog_field_list);

            // Run through the remaining list
            // to locate the next test string.
            while (ctr < file_lines) {
                std::string file_line = file_line_list[ctr];
                if (file_line.compare(0, expected_prefix.size(),
                        expected_prefix) == 0)
                    break;

                ctr += 1;
            }

            // If we reached the end of the file, terminate the search.
            if (ctr == file_lines) {
                std::cout << "Expected output has fewer test strings than "
                    "program output!" << std::endl;
                return false;
            }

            string_list_t file_field_list;
            std::string file_line = file_line_list[ctr];
            split(file_line, ':', file_field_list);

            // Advance to the next line in the file.
            ctr += 1;

            size_t prog_fields = prog_field_list.size();
            size_t file_fields = file_field_list.size();

            // Quick sanity check.
            assert (prog_fields >= 2);
            assert (file_fields >= 2);

            if (prog_fields != file_fields) {
                std::cout << "Field length mismatch between program output and "
                    "expected output: " << prog_line << " v/s " <<
                    file_line << "!" << std::endl;
                return false;
            }

            // Start from index 1, because no point in
            // comparing the test identifier [expected_prefix].
            for (int i=1; i<prog_fields; i++) {
                if (prog_field_list[i] != file_field_list[i] &&
                        file_field_list[i] != "?") {
                    std::cout << "Field mismatch between program output and "
                        "expected output: " << prog_field_list[i] << " v/s " <<
                        file_field_list[i] << "!" << std::endl;
                    return false;
                }
            }
        }
    }

    // Are there any expected outputs that
    // we did not see in the output?
    if (ctr == file_lines || (ctr < file_lines &&
            file_line_list[ctr].compare(0, expected_prefix.size(),
                expected_prefix) != 0)) {
        return true;
    }
}

static void populate_args(string_list_t& args, std::string& input_file,
        std::string& output_file, string_list_t* special_args) {
    // Standard compiler arguments.
    args.push_back("cc");
    args.push_back(input_file);
    args.push_back("-o");
    args.push_back(output_file);
    args.push_back("-lstdc++");

    // Path to mock libmrt files.
    std::string mock_include_path = get_src_directory() + "/libmrt";
    args.push_back("-I" + mock_include_path);

    std::string mock_library_path = get_build_directory() + "/libmrt";
    args.push_back("-L" + mock_library_path);
    args.push_back("-lmrt");

    if (special_args) {
        args.insert(args.end(), special_args->begin(), special_args->end());
    }
}

static std::string get_process_stderr(std::string& binary_name) {
    int err_pipe[2]; 
    pipe(err_pipe); 

    // Create args.
    char arg0[1024];
    snprintf (arg0, 1024, "%s", binary_name.c_str());
    char *args[] = { arg0, NULL };

    switch (fork()) {
        case -1:
            return "";

        case 0:
            // Child.
            close(err_pipe[0]);

            // Duplicate file descriptors.
            if (dup2(err_pipe[1], 1) < 0) {
                perror("Could not duplicate descriptor");
                exit(-1); 
            }

            if (dup2(err_pipe[1], 2) < 0) {
                perror("Could not duplicate descriptor");
                exit(1); 
            } 

            // Run the binary.
            if (execv(arg0, args) == -1){ 
                std::cout << "Could not execute binary file: " << binary_name <<
                    std::endl; 
                perror("execv"); 
                exit(2); 
            }

            // Done!
            close(err_pipe[1]);
            exit(0); 
    }

    // Wait until child process finishes executing.
    wait(NULL); 

    fcntl(err_pipe[0], F_SETFL, O_NONBLOCK | O_ASYNC); 

    const int buffer_len = 1024;
    char buffer[buffer_len];
    std::string str_stderr;

    // Read child process's output.
    while (true) { 
        ssize_t read_count = read(err_pipe[0], buffer, buffer_len-1);
        if (read_count <= 0 || read_count >= buffer_len)
            break; 

        buffer[read_count] = 0;
        str_stderr += buffer;
    }

    // Cleanup and return.
    close(err_pipe[0]); 
    close(err_pipe[1]);
    return str_stderr;
}

static void split(const std::string& string, char delim, string_list_t& list) {
    std::stringstream stream(string);
    std::string item;

    while (std::getline(stream, item, delim)) {
        list.push_back(item);
    }
}

static std::string get_file_contents(std::string& filename) {
    std::string line, contents;
    std::ifstream fileobj(filename.c_str());

    if (fileobj.is_open()) {
        while (getline(fileobj,line))
            contents += line + "\n";

        fileobj.close();
    }

    return contents;
}
