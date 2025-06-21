import signal
import sys
import platform

# install any libraries in the py-ext virtual-env (or any other if you modify the exact setup)

# See "Start here" below. The purpose of this script is to support external preprocessing algorithms to
# transform the input text into a form that is easier and more reliable to parse in lingua.py.
# This could potentially increase the expressive range of spoken or typed input, so long as it can be converted into
# such a simpler form.

def run():
    sys.stderr.write("External language pre-processing script started with Python" + platform.python_version() + "\n"
        "Ready!\n")
    sys.stderr.flush()

    while 1:
        line = sys.stdin.readline()

        sys.stdout.flush()

        while line:
            if line == "exit":
                sys.stderr.write("exiting external script")
                sys.stderr.flush()
                sys.exit(0)

            mod_count_line = sys.stdin.readline()
            is_finished_line = sys.stdin.readline()
            context_line = sys.stdin.readline()
            text_line = sys.stdin.readline()

            #sys.stderr.write("<(extern)\n" + line + mod_count_line + is_finished_line + context_line + text_line + "\n" + ">")
            sys.stderr.flush()

            ID          = int(line.strip())
            mod_count   = int(mod_count_line.strip())
            is_finished = int(is_finished_line.strip())
            context     = context_line.strip()

            # these should be pass-through
            if context == "synonym" or context == "reset":
                sys.stdout.write(line + mod_count_line + is_finished_line + context_line + text_line)
                sys.stdout.flush()

                line = sys.stdin.readline()
                continue

            # the raw input text
            text = text_line.strip()

            # # # # # # # # # # # # # # # # # # # # # # # #
            # Start here 
            #   (modify text with custom pre-processing,
            #    e.g. simplify)! 
            #   - Currently pass-through
            #     (sends data unchanged to the main process)
            # - - - - - - - - - - - - - - - - - - - - - - -
            # 
            # - Process the raw text input (e.g. simplify)
            # - For debugging, write to sys.stderr
            if is_finished:
                # Do something with the completed text when you confirm that the input is ready.
                # Usually you want to take this path.
                #

                # A silly example might be to append "twice" to the end of every sentence.
                # by inserting the word before the end-punctuation.
                # text = text[0:len(text)-1] + " twice" + text[-1]

                # A more practical example might be to rewrite or simplify the sentence
                # by using any techniques you want to improve the reliability of lingua.py's final parse.
                pass
            else:
                # Do something with the input that is underway / streamed
                # You might need to leave this alone if any techniques you'd like to use require the full input sentence
                # Or if the techniques you'd like to transform the sentence are costly.
                pass

            # End pre-processing here (store result in the variable: text)
            #
            # # # # # # # # # # # # # # # # # # # # # # # #

            # write to sys.stdout and flush to send to final processing stage (text_line is either the original or modified)
            sys.stdout.write(line + mod_count_line + is_finished_line + context_line + text + "\n")
            sys.stdout.flush()

            line = sys.stdin.readline()


if __name__ == "__main__":
    run()
else:
    # local testing here
    pass