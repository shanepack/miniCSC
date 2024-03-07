import os
import argparse
import json

VERBOSE = False
HTML = False


def log(*format: str):
    if VERBOSE:
        for msg in format:
            print(msg, end=" ")
        print()


def parse_txt(file_path: str, header_data, tmb_data):
    file = open(file_path, "r")
    lines = file.readlines()

    # Parse lines
    for line in lines:
        # Header data stuff
        if "Start" in line and "Start" in header_data:
            header_data["Start"] = lines[lines.index(line) + 1].strip()
        elif "Stop" in line and "Stop" in header_data:
            header_data["Stop"] = lines[lines.index(line) + 1].strip()
        elif (
            "Data files" in line and "Data files" in header_data
        ):  # Generates urls to plots and raw files
            url = lines[lines.index(line) + 1].strip()
            tmp_data = url.split("/")
            if not tmp_data[0].startswith("http"):
                # Removing unneeded parts
                tmp_data.remove("daq")
                tmp_data.remove("current")
                tmp_data[0] = tmp_data[0][:-1]
                tmp_data.insert(0, "http:/")
                url = "/".join(tmp_data)
            header_data["Data files"] = url
            tmp_plots = url.split("/")
            tmp_plots.insert(4, "Tests_results")
            tmp_plots.insert(5, "Test_27_Cosmics")
            tmp_plots[-1] = tmp_plots[-1][:-4] + ".plots"
            tmp_plots.append("browse.html")
            header_data["Data plots"] = "/".join(tmp_plots)

        # Data stored in the tmb_dump
        # ? Move to other function?
        else:
            # Only works if line has no other numbers
            for key in tmb_data:
                try:
                    splitStr = line.split()
                    if splitStr[0] == key + ":":
                        value = float(splitStr[-1]) / 10
                        tmb_data[key] = value

                except IndexError:
                    continue
    file.close()
    return header_data | tmb_data


def get_run_num(file):
    return int(file.split("-")[0][1:])


# TODO: doing some funky stuff to get the right data
def generate_elog(files: list[str]):
    buffer = ""
    # Initialize data structure
    header_data = {
        "Start": "",
        "Stop": "",
        "Data files": "",
        "Data plots": "",
    }
    tmb_data = {
        "0ALCT": 0,
        "20CLCT": 0,
        "32TMB": 0,
    }
    if HTML:
        log("Inserting HTML styling")
        buffer += (
            '<pre>\n<strong><span style="font-size:large">Title\n\n</span></strong>'
        )
    for file in files:
        file_data = parse_txt(file, header_data, tmb_data)
        # for key, value in file_data.items():
        #     buffer += f"{key}: {value}\n"
        buffer += "Start: " + file_data["Start"] + "\n"
        buffer += "Stop: " + file_data["Stop"] + "\n"
        buffer += "0ALCT: " + str(file_data["0ALCT"]) + "\n"
        buffer += "20CLCT: " + str(file_data["20CLCT"]) + "\n"
        buffer += "32TMB: " + str(file_data["32TMB"]) + "\n"

    buffer += "</pre>"
    # buffer += "</span></strong></pre>"
    return buffer


def generate_csv(files: list[str]):
    buffer = ""
    header_data = {
        "Run": 0,
        "Source": "",
        "Data files": "",
        "Data plots": "",
        "Start": "",
        "Stop": "",
    }
    tmb_data = {
        "0ALCT": 0,
        "20CLCT": 0,
        "32TMB": 0,
    }
    for file in files:
        data = parse_txt(file, header_data, tmb_data)
        # print(json.dumps(file_data, indent=2))
        file_split = file.split("-")
        buffer += file_split[0] + ","  # run num
        buffer += file_split[4] + ","  # source
        buffer += file_split[3] + ","  # hole num
        for key in tmb_data:  # tmb_data values
            buffer += str(data[key]) + ","
        buffer += data["Data files"] + ","
        buffer += data["Data plots"] + ","
        buffer += data["Start"][:-4] + ","  # Removing UTC unit
        buffer += data["Stop"][:-4] + "\n"  # Removing UTC unit
    return buffer


def process_directory(directory: str, num: int):
    log(f"Processing {num} files from: {directory}")
    files = os.listdir(directory)
    new_files = []
    count = 0
    for file in files:
        if (
            file.split("-")[0][1:].isdigit()
            and file.endswith(".txt")
            and file.startswith("r")
        ):
            if count >= num:
                break
            count += 1
            log(f"Will process {file}")
            new_files.append(file)

    return sorted(new_files, key=get_run_num)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Increase script verbosity"
    )
    # parser.add_argument(
    #     "-e", "--elog", action="store_true", help="Generate ELOG output"
    # )
    # parser.add_argument("-c", "--csv", action="store_true", help="Generate CSV output")
    parser.add_argument(
        "-n",
        "--num",
        type=int,
        default=100,
        metavar="num",
        help="Specify number of files to process (default: 100)",
    )
    parser.add_argument("--html", action="store_true", help="Export with HTML styling")
    args = parser.parse_args()
    VERBOSE = args.verbose
    # ELOG = args.elog
    HTML = args.html
    # CSV = args.csv

    # Update these paths according to your local setup
    directory = os.path.dirname(__file__)
    output_dir = os.path.join(directory, "output")
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    elog_out = os.path.join(output_dir, "elog_out.txt")
    csv_out = os.path.join(output_dir, "csv_out.txt")

    log(f"Parsing directory: {directory}")
    files = process_directory(directory, args.num)

    # Generate elog
    # if ELOG:
    log("Generating Elog at: ", elog_out)
    elog_data = generate_elog(files)
    elog_file = open(elog_out, "w")
    elog_file.write(elog_data)
    elog_file.close()

    # Generate CSV
    # if CSV:
    log("Generating CSV at: ", csv_out)
    csv_data = generate_csv(files)
    csv_file = open(csv_out, "w")
    csv_file.write(csv_data)
    csv_file.close()
