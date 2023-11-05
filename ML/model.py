import os
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score
import joblib

# Get the parent directory of the current working directory
parent_directory = os.path.dirname(os.getcwd())

# Specify the directory path
directory_path = os.path.join(parent_directory, 'ML\\dataset\\NVD')

# Collect code snippets and their labels from files
X = []
y = []
z = []

for filename in os.listdir(directory_path):
    if "PATCHED" in filename:
        label = 0  # 0 for not vulnerable
        vulnerability_name = filename.split('PATCHED_')[1]
    elif "VULN" in filename:
        label = 1  # 1 for vulnerable
        vulnerability_name = filename.split('VULN_')[1]

    with open(os.path.join(directory_path, filename), 'r') as file:
        code = file.read()
        X.append(code)
        y.append(label)
        z.append(vulnerability_name)

# Feature Engineering: Convert code snippets into numerical features
vectorizer = CountVectorizer()
X_features = vectorizer.fit_transform(X)

# Initialize and train the Decision Tree Classifier
clf = DecisionTreeClassifier()
clf.fit(X_features, y)

# Save the model for later use
joblib.dump(clf, 'vulnerability_detection_model.joblib')
joblib.dump(vectorizer, 'vectorizer.joblib')
joblib.dump(z, 'vulnerability_names.joblib')