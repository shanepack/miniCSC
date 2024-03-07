import os
import json


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
    for file in files:
        file_data = parse_txt(file, header_data, tmb_data)
        buffer += f"File: {file}\n"
        for key, value in file_data.items():
            buffer += f"{key}: {value}\n"
        buffer += "\n"
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
        buffer += str(data["0ALCT"]) + ","
        buffer += str(data["20CLCT"]) + ","
        buffer += str(data["32TMB"]) + ","
        buffer += data["Data files"] + ","
        buffer += data["Data plots"] + ","
        buffer += data["Start"][:-4] + ","  # Removing UTC unit
        buffer += data["Stop"][:-4] + "\n"  # Removing UTC unit
    return buffer


def process_directory(directory):
    files = os.listdir(directory)
    new_files = []
    for file in files:
        if (
            file.split("-")[0][1:].isdigit()
            and file.endswith(".txt")
            and file.startswith("r")
        ):
            print(f"Will process {file}")
            new_files.append(file)

    return sorted(new_files, key=get_run_num)


if __name__ == "__main__":
    # Update these paths according to your local setup
    directory = os.path.dirname(__file__)
    elog_out = os.path.join(directory, "output/elog_out.txt")
    csv_out = os.path.join(directory, "output/csv_out.txt")

    files = process_directory(directory)

    # Generate elog
    print("Generating Elog at: ", elog_out)
    elog_data = generate_elog(files)
    elog_file = open(elog_out, "w")
    elog_file.write(elog_data)
    elog_file.close()

    # Generate CSV
    print("Generating CSV at: ", csv_out)
    csv_data = generate_csv(files)
    csv_file = open(csv_out, "w")
    csv_file.write(csv_data)
    csv_file.close()
