import os


def parse_elog_txt(file_path: str):
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
            # if "http" not in tmpData[0]:
            if not tmpData[0].startswith("http"):
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

        else:
            for key in ["0ALCT", "13CLCT", "32TMB"]:
                try:
                    splitStr = line.split()
                    if splitStr[0] == key + ":":
                        value = float(splitStr[-1]) / 10
                        data[key] = value

                except IndexError:
                    continue
    file.close()
    return data


def parse_csv_txt(filepath: str) -> str:
    file = open(filepath, "r")
    return ""


def get_run_num(file):
    return int(file.split("-")[0][1:])


def generate_elog(files: list[str]):
    buffer = ""
    for file in files:
        file_data = parse_elog_txt(file)
        buffer += f"File: {file}\n"
        for key, value in file_data.items():
            buffer += f"{key}: {value}\n"
        buffer += "\n"
    return buffer


def generate_csv(files: list[str]):
    buffer = ""
    for file in files:
        file_data = parse_csv_txt(file)
        print(file_data)
    pass


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
    output_file = os.path.join(directory, "output/compiled_data_corrected.txt")

    files = process_directory(directory)

    # Generate elog
    print("Generating Elog at: ", output_file)
    elog_data = generate_elog(files)
    elog_file = open(output_file, "w")
    elog_file.write(elog_data)
    elog_file.close()

    generate_csv(files)
