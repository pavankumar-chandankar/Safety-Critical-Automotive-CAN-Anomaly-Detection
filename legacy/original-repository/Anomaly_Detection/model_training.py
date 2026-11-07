import logging
from sklearn.ensemble import IsolationForest
import joblib  # For saving the model
from data_preprocessing import preprocess_data
from data_acquisition import load_car_evaluation_data

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def train_prophet_model(df, target_column):
    """
    Trains a Prophet model for time series forecasting.

    Args:
        df (pd.DataFrame): The preprocessed DataFrame.
        target_column (str): The name of the target column.

    Returns:
        Prophet: The trained Prophet model.
    """
    logging.info("Training Prophet model...")
    model = Prophet()
    df = df.reset_index().rename(columns={'Datetime': 'ds', target_column: 'y'})
    model.fit(df)
    logging.info("Prophet model trained.")
    return model

def save_model(model, filepath="prophet_model.joblib"):
    """
    Saves the trained model to a file.

    Args:
        model (Prophet): The trained Prophet model.
        filepath (str): The path to save the model.
    """
    logging.info(f"Saving model to: {filepath}")
    joblib.dump(model, filepath)
    logging.info("Model saved.")

if __name__ == "__main__":
    transport_data = load_public_transport_data()
    if transport_data is not None:
        target_column = 'Count'  # Replace with the actual target column name
        preprocessed_data = preprocess_data(transport_data.copy(), target_column)
        model = train_prophet_model(preprocessed_data, target_column)
        save_model(model)
        logging.info("Model training and saving completed.")
    else:
        logging.error("Model training failed due to data loading error.")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def train_isolation_forest(X_train):
    """
    Trains an Isolation Forest model.

    Args:
        X_train (pd.DataFrame): The training data.

    Returns:
        IsolationForest: The trained Isolation Forest model.
    """
    logging.info("Training Isolation Forest model...")
    model = IsolationForest(contamination=0.1, random_state=42)  # Adjust contamination as needed
    model.fit(X_train)
    logging.info("Isolation Forest model trained.")
    return model

def save_model(model, filepath="isolation_forest_model.joblib"):
    """
    Saves the trained model to a file.

    Args:
        model (IsolationForest): The trained model.
        filepath (str): The path to save the model.
    """
    logging.info(f"Saving model to: {filepath}")
    joblib.dump(model, filepath)
    logging.info("Model saved.")

if __name__ == "__main__":
    car_data = load_car_evaluation_data()
    if car_data is not None:
        X_train, X_test, y_train, y_test = preprocess_data(car_data.copy())
        model = train_isolation_forest(X_train)
        save_model(model)
        logging.info("Model training and saving completed.")
    else:
        logging.error("Model training failed due to data loading error.")
