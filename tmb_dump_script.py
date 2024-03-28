import os
import ntpath
import argparse
from pprint import pprint

# Global Argument Flags
VERBOSE = False
HTML = False
REM_DUP = False

DUMP_TIME = 10  # seconds


def log(*format: str):
    if VERBOSE:
        for msg in format:
            print(msg, end=" ")
        print()


def parse_title(file_path: str):
    head, tail = ntpath.split(file_path)
    file_title = tail or ntpath.basename(head)
    log("Parsing file title:", file_title)
    title_split = file_title.split("-")
    data = {
        "Run": title_split[0],
        "Chamber": title_split[1],
        "Layers": title_split[2],
        "Layer Pos": "",
        "Hole": title_split[3],
        "Source": "NA",
        "Test": title_split[5],
        "HV": title_split[6],
        "Events": title_split[7][:-9],
    }
    # TODO: make dynamically detect
    match data["Layers"]:
        case "L1":
            data["Layer Pos"] = "Bottom Layer"
        case "L2":
            data["Layer Pos"] = "Top Layer"
        case "L1+2":
            data["Layer Pos"] = "Both Layers"
        case "L1+L2":
            data["Layer Pos"] = "Both Layers"

    # Rad source formatting
    source = title_split[4].title()
    sym = ""
    # Get element name
    for ch in source:
        if not ch.isdigit():
            sym += ch

    weight = ""
    # Get atomic weight
    for ch in source:
        if ch.isdigit():
            weight += ch

    data["Source"] = sym + "-" + weight
    return data


def parse_txt(file_path: str):
    data = {
        "Start": "Not Measured",  # Default to "NA"
        "Stop": "Not Measured",
        "Data files": "NA",
        "Data plots": "NA",
        "Pressure": "Not Measured",  # You may want to keep this specific default
        "Temp": "Not Measured",
        "TMB Dump": [],
    }
    #file = open(file_path, "r")
    #lines = file.readlines()
    
    with open(file_path, "r") as file:
        lines = file.readlines()
        tmb_start = 1024
    
    for i, line in enumerate(lines):
        # Skip empty or comment lines
        if len(line) < 2 or line.startswith("#"):
            continue
        
        if "Start" in line:
            # Ensure there's a next line and it's valid
            next_line = lines[i + 1].strip() if i + 1 < len(lines) else ""
            data["Start"] = next_line if next_line else "Not Measured"
            
        elif "Stop" in line:
            # Ensure there's a next line and it's valid
            next_line = lines[i + 1].strip() if i + 1 < len(lines) else ""
            data["Stop"] = next_line if next_line else "Not Measured"
            
        elif "Pressure" in line:
            data["Pressure"] = lines[lines.index(line) + 1].strip() + " mbar"
        elif "Temp" in line:
            data["Temp"] = lines[lines.index(line) + 1].strip() + " C"
        elif "Data files" in line:  # Generates urls to plots and raw files
            url = lines[lines.index(line) + 1].strip()
            tmp_data = url.split("/")
            if not tmp_data[0].startswith("http"):
                # Removing unneeded parts
                tmp_data.remove("daq")
                tmp_data.remove("current")
                tmp_data[0] = tmp_data[0][:-1]
                tmp_data.insert(0, "http:/")
            # Checking for weird other colon in first, second, or third slot
            for i in range(3):
                if "cern.ch:" in tmp_data[i]:
                    tmp_data[i] = tmp_data[i].replace(":", "")
            url = "/".join(tmp_data)
            data["Data files"] = url
            tmp_plots = url.split("/")
            tmp_plots.insert(4, "Tests_results")
            tmp_plots.insert(5, "Test_27_Cosmics")
            tmp_plots[-1] = tmp_plots[-1][:-4] + ".plots"
            tmp_plots.append("browse.html")
            data["Data plots"] = "/".join(tmp_plots)

        # Data stored in the tmb_dump
        else:
            if "Counters" in line:
                tmb_start = lines.index(line) + 2
            if lines.index(line) < tmb_start:
                continue
            try:
                line_split = line.split()
                data["TMB Dump"].append(int(line_split[-1]))
            except IndexError:
                log(f"Index Error @ file line: {lines.index(line)}")
                continue
            except ValueError:
                # Mainly catches 36TMB issue
                # log(f"Value Error @ file line: {lines.index(line)}")
                line_split = line.split()
                value = int([x for x in line_split[-1] if x.isdigit()][0])
                data["TMB Dump"].append(value)
                continue

    file.close()
    return data


def parse_files(files: list[str]) -> list[dict]:
    print(f"Extracting data from {len(files)} files.")
    data_list = [{"title_data": {}, "file_data": {}} for i in range(len(files))]
    i = 0
    for file in files:
        data_list[i]["title_data"] = parse_title(file)
        data_list[i]["file_data"] = parse_txt(file)
        i += 1
    return data_list


def get_run_num(file):
    return int(file.split("-")[0][1:])


def generate_elog(files_data: list[dict]):
    buffer = ""
    for data in files_data:
        title_data = data["title_data"]
        file_data = data["file_data"]
        title = f"{title_data['Layers']} ({title_data['Layer Pos']}) - {title_data['HV'][:-1]} V "
        if title_data["Source"] == "Na-":
            title += "(No Radiation Source)"
        else:
            title += f"({title_data['Source']} @ Hole #{title_data['Hole'][1:]})"
        title += f" {title_data['Events']} Events - {title_data['Run'].capitalize()}"
        if HTML:
            log("Inserting HTML styling")
            buffer += f'<pre>\n<strong><span style="font-size:large">{title}\n\n</span></strong>'
        else:
            buffer += f"{title}\n"
        # for key, value in file_data.items():
        #     buffer += f"{key}: {value}\n"
        buffer += "Start: " + file_data["Start"] + "\n"
        buffer += "Stop: " + file_data["Stop"] + "\n"
        buffer += "Pressure: " + file_data["Pressure"] + "\n"
        buffer += "Temperature: " + file_data["Temp"] + "\n"
        tmb_dump = file_data["TMB Dump"]
        buffer += "0ALCT: " + (str(tmb_dump[0] / DUMP_TIME) + " Hz" if len(tmb_dump) > 0 else "Not Measured") + "\n"
        buffer += "20CLCT: " + (str(tmb_dump[20] / DUMP_TIME) + " Hz" if len(tmb_dump) > 20 else "Not Measured") + "\n"
        buffer += "32TMB: " + (str(tmb_dump[32] / DUMP_TIME) + " Hz" if len(tmb_dump) > 32 else "Not Measured") + "\n"
        if HTML:
            buffer += f'<a href="{file_data["Data plots"]}">Link to plots</a>\n'
            buffer += (
                f'<a href="{file_data["Data files"]}">Link to raw data files</a>\n'
            )
            buffer += "</pre>"
        else:
            buffer += file_data["Data plots"] + "\n"
            buffer += file_data["Data files"] + "\n"
        buffer += "\n"
    # buffer += "</span></strong></pre>"
    return buffer


def generate_csv(files_data: list[dict]):
    buffer = ""
    previous_layers = ""
    previous_voltage = ""
    for data in files_data:
        title_data = data["title_data"]
        file_data = data["file_data"]
        buffer += title_data["Run"] + ","  # run num

        # Handling duplicate values if required
        if REM_DUP:
            if title_data["Layers"] != previous_layers:
                buffer += title_data["Layers"]
            else:
                buffer += '"'
            buffer += ","
            previous_layers = title_data["Layers"]

            if title_data["HV"] != previous_voltage:
                buffer += title_data["HV"][:-1]
            else:
                buffer += '"'
            buffer += ","
            previous_voltage = title_data["HV"]
        else:
            buffer += title_data["Layers"] + ","
            buffer += title_data["HV"] + ","

        buffer += "NA," if title_data["Source"] == "Na-" else title_data["Source"] + ","  # source
        buffer += title_data["Hole"][1:] + ","  # hole num
        
        # Safely access TMB Dump values or use "NA"
        tmb_dump = file_data["TMB Dump"]
        buffer += (str(tmb_dump[0] / DUMP_TIME) + "," if len(tmb_dump) > 0 else "Not Measured,")
        buffer += (str(tmb_dump[20] / DUMP_TIME) + "," if len(tmb_dump) > 20 else "Not Measured,")
        buffer += (str(tmb_dump[32] / DUMP_TIME) + "," if len(tmb_dump) > 32 else "Not Measured,")

        buffer += (file_data["Data files"] if file_data["Data files"] != "NA" else "NA") + ","
        buffer += (file_data["Data plots"] if file_data["Data plots"] != "NA" else "NA") + ","
        buffer += (file_data["Start"][:-4] + "," if file_data["Start"] != "Not Measured" else "Not Measured,")  # If time is not inputted, it will be "Not Measured"
        buffer += (file_data["Stop"][:-4] + "," if file_data["Stop"] != "Not Measured" else "Not Measured,")  
        buffer += title_data["Events"] + ","
        buffer += file_data["Pressure"].replace(" mbar", "") + "," # Removing 'mbar' unit from csv output
        buffer += file_data["Temp"].replace(" C", "") + "," # Removing ' C' unit from csv output
        buffer += "\n"
    return buffer


def parse_directory(directory: str, num: int) -> list[str]:
    """Parses directory to generate a list of files to extract data from.

    Args:
        directory (str): Directory to parse.
        num (int): Number of files to process.

    Returns:
        list[str]: List of files in directory to extract from
    """

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
    descr = "Reads a text file containing CSC run statistics and generates an Elog and csv for documentation.\n"
    parser = argparse.ArgumentParser(
        prog="TMB Parser Script",
        description=descr,
    )
    # parser.add_argument("-c", "--csv", action="store_true", help="Generate CSV output")
    # parser.add_argument(
    #     "-e", "--elog", action="store_true", help="Generate ELOG output"
    # )
    parser.add_argument(
        "-d",
        "--remove-dup",
        action="store_true",
        help="Remove duplicate entries from the CSV output",
    )
    parser.add_argument(
        "-n",
        "--num",
        type=int,
        default=100,
        metavar="num",
        help="Specify number of files to process (default: 100)",
    )
    parser.add_argument(
        "--html", action="store_true", help="Export Elog with HTML styling"
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Increase script verbosity"
    )
    parser.add_argument("--version", action="version", version="%(prog)s 0.1")
    args = parser.parse_args()

    VERBOSE = args.verbose
    # ELOG = args.elog
    REM_DUP = args.remove_dup
    HTML = args.html
    # CSV = args.csv

    directory = os.path.dirname(__file__)
    output_dir = os.path.join(directory, "output")
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    elog_out = os.path.join(output_dir, "elog_out.txt")
    csv_out = os.path.join(output_dir, "csv_out.txt")

    files = parse_directory(directory, args.num)
    if len(files) == 0:
        print(f"Cannot find files in {directory}. Stopping")
        exit(1)

    data = parse_files(files)

    # Generate elog
    # if ELOG:
    print("Generating Elog at: ", elog_out)
    elog_data = generate_elog(data)
    with open(elog_out, "w", encoding='utf-8') as elog_file:
        elog_file.write(elog_data)
    elog_file.close()

    # Generate CSV
    # if CSV:
    print("Generating CSV at: ", csv_out)
    csv_data = generate_csv(data)
    with open(csv_out, "w", encoding='utf-8') as csv_file:
        csv_file.write(csv_data)
    csv_file.close()
