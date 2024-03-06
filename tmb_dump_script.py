import os

def parse_txt_file(file_path):
    # Initialize data structure
    data = {
        'Start': '',
        'Stop': '',
        'Data files': '',
        '0ALCT': 0,
        '13CLCT': 0,
        '32TMB': 0,
    }

    file = open(file_path, 'r')
    lines = file.readlines()
    
    # Parse lines
    for line in lines:
        if 'Start' in line:
            data['Start'] = lines[lines.index(line) + 1].strip()
        elif 'Stop' in line:
            data['Stop'] = lines[lines.index(line) + 1].strip()
        elif 'Data files' in line:
            data['Data files'] = lines[lines.index(line) + 1].strip()
        else:
            for key in ['0ALCT', '13CLCT', '32TMB']:
                try:
                    splitStr = line.split()
                    if splitStr[0] == key + ':':
                        value = float(splitStr[-1]) / 10
                        data[key] = value

                except:
                    continue
    return data

def process_directory(directory_path, output_file_path):
    # Write to the output file
    with open(output_file_path, 'w', encoding='utf-8') as output_file:
        print(f'writing to: {output_file_path}')
        for filename in sorted(os.listdir(directory_path)):
            if filename.endswith('.txt'):
                filepath = os.path.join(dirname, filename)
                file_data = parse_txt_file(filepath)
                output_file.write(f"File: {filename}\n")
                for key, value in file_data.items():
                    output_file.write(f"{key}: {value}\n")
                output_file.write("\n")  # Add a newline for readability between files

if __name__ == '__main__':
    # Update these paths according to your local setup
    dirname = os.path.dirname(__file__)
    directory_path = dirname
    output_file_path = os.path.join(dirname, 'output/compiled_data_corrected.txt')

    process_directory(directory_path, output_file_path)
