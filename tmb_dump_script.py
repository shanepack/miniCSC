import os


def parse_txt_file(file_path):
    # Initialize data structure
    data = {
        "Start": "",
        "Stop": "",
        "Data files": "",
        "Data plots": "",
        "0ALCT": 0,
        "13CLCT": 0,
        "32TMB": 0,
    }

    file = open(file_path, "r")
    lines = file.readlines()

    # Parse lines
    for line in lines:
        if "Start" in line:
            data["Start"] = lines[lines.index(line) + 1].strip()
        elif "Stop" in line:
            data["Stop"] = lines[lines.index(line) + 1].strip()
        elif "Data files" in line:
            url = lines[lines.index(line) + 1].strip()
            tmpData = url.split("/")
            if "http" not in tmpData[0]:
                print("Refactoring url")
                # Removing unneeded parts
                tmpData.remove("daq")
                tmpData.remove("current")
                tmpData[0] = tmpData[0][:-1]
                tmpData.insert(0, "http:/")
                url = "/".join(tmpData)
            data["Data files"] = url
            tmpPlots = url.split("/")
            tmpPlots.insert(4, "Tests_results")
            tmpPlots.insert(5, "Test_27_Cosmics")
            tmpPlots[-1] = tmpPlots[-1][:-4] + ".plots"
            tmpPlots.append("browse.html")
            data["Data plots"] = "/".join(tmpPlots)
            # Data files: http://emuchick2.cern.ch/data/csc_00000001_EmuRUI01_STEP_27s_000_240306_122210_UTC.raw

        else:
            for key in ["0ALCT", "13CLCT", "32TMB"]:
                try:
                    splitStr = line.split()
                    if splitStr[0] == key + ":":
                        value = float(splitStr[-1]) / 10
                        data[key] = value

                except IndexError:
                    continue
    return data


<<<<<<< Updated upstream
def process_directory(directory_path, output_file_path):
    # Write to the output file
    with open(output_file_path, "w", encoding="utf-8") as output_file:
        print(f"writing to: {output_file_path}")
        for filename in sorted(os.listdir(directory_path)):
            if filename.endswith(".txt"):
                filepath = os.path.join(dirname, filename)
                file_data = parse_txt_file(filepath)
                output_file.write(f"File: {filename}\n")
                for key, value in file_data.items():
                    output_file.write(f"{key}: {value}\n")
                output_file.write("\n")  # Add a newline for readability between files
=======
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
        for key in tmb_data:  # tmb_data values
            buffer += str(data[key]) + ","
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
>>>>>>> Stashed changes


if __name__ == "__main__":
    # Update these paths according to your local setup
    dirname = os.path.dirname(__file__)
    directory_path = dirname
    output_file_path = os.path.join(dirname, "output/compiled_data_corrected.txt")

    process_directory(directory_path, output_file_path)
