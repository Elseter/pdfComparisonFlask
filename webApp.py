from flask import Flask, request, render_template, redirect, url_for, send_from_directory, flash
from werkzeug.utils import secure_filename
import os
import subprocess
import csv


app = Flask(__name__)
app.secret_key = 'supersecretkey'  # Required for flashing messages
app.config['UPLOAD_FOLDER'] = 'build/uploads'
comparison_dir = 'output'
app.config['ALLOWED_EXTENSIONS'] = {'pdf'}
if not os.path.exists(app.config['UPLOAD_FOLDER']):
    os.makedirs(app.config['UPLOAD_FOLDER'])

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in app.config['ALLOWED_EXTENSIONS']

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        # check if the post request has the file part
        file = request.files.get('file')
        if not file:
            return "No file part", 400
        if file.filename == '':
            return "No selected file", 400
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(file_path)
            return render_template('index.html')
        else:
            return 'Invalid file type', 400
    return render_template('index.html')

@app.route('/saved_files')
def saved_files():
    files = os.listdir(app.config['UPLOAD_FOLDER'])
    return render_template('saved_files.html', files=files)

@app.route('/rename_file', methods=['POST'])
def rename_file():
    old_name = request.form['filename']
    new_name = request.form['new_name']
    old_path = os.path.join(app.config['UPLOAD_FOLDER'], old_name)
    new_path = os.path.join(app.config['UPLOAD_FOLDER'], new_name)
    if os.path.exists(old_path):
        os.rename(old_path, new_path)
    return redirect(url_for('saved_files'))

@app.route('/delete_file', methods=['POST'])
def delete_file():
    filename = request.form['filename']
    file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    if os.path.exists(file_path):
        os.remove(file_path)
    return redirect(url_for('saved_files'))

@app.route('/view_file/<filename>')
def view_file(filename):
    if allowed_file(filename):
        return send_from_directory(app.config['UPLOAD_FOLDER'], filename)
    else:
        return "File not found or invalid file type", 404

@app.route('/compare_files', methods=['GET', 'POST'])
def compare_files():
    files = os.listdir(app.config['UPLOAD_FOLDER'])
    if request.method == 'POST':
        pdf1 = request.form['pdf1']
        pdf2 = request.form['pdf2']
        similarity_score = request.form['similarity_score']
        pdf1_path = os.path.join(app.config['UPLOAD_FOLDER'], pdf1)
        pdf2_path = os.path.join(app.config['UPLOAD_FOLDER'], pdf2)
        subprocess.run(['build/comparePDFs', pdf1_path, pdf2_path, similarity_score])
    return render_template('compare_files.html', files=files)

@app.route('/comparison_files')
def comparison_files():
    files = os.listdir(comparison_dir)
    return render_template('comparison_files.html', files=files)

@app.route('/view_comparison_file/<filename>')
def view_comparison_file(filename):
    if filename.endswith('.csv'):  # Only allow viewing CSV files
        file_path = os.path.join(comparison_dir, filename)
        if os.path.exists(file_path):
            with open(file_path, 'r') as f:
                csv_reader = csv.reader(f, delimiter='^')
                data = []
                encountered_questions = {}
                for row in csv_reader:
                    if len(row) == 5:
                        question1, page1, question2, page2, similarity = row
                        if question1 not in encountered_questions:
                            encountered_questions[question1] = True
                            entry = {
                                'question1': question1,
                                'question1_page': page1,
                                'matches': []
                            }
                            data.append(entry)
                        entry = next((entry for entry in data if entry['question1'] == question1), None)
                        entry['matches'].append({
                            'question2': question2,
                            'question2_page': page2,
                            'similarity': similarity
                        })

            # Sort matches for each question based on similarity (descending order)
            for entry in data:
                entry['matches'] = sorted(entry['matches'], key=lambda x: float(x['similarity']), reverse=True)
            return render_template('view_comparison_file.html', filename=filename, data=data)
        else:
            return "File not found", 404
    else:
        return "Invalid file type", 400

if __name__ == '__main__':
    os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)  # Ensure upload folder exists
    app.run(host='0.0.0.0', port=3030)