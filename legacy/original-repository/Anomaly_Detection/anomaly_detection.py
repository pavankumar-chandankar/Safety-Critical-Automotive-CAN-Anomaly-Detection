import logging
from prophet import Prophet
import joblib
import pandas as pd
from sklearn.metrics import mean_absolute_error, mean_squared_error
from data_preprocessing import preprocess_data
from data_acquisition import load_public_transport_data

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def load_model(filepath="prophet_model.joblib"):
    """
    Loads the trained Prophet model from a file.

    Args:
        filepath (str): The path to the saved model.

    Returns:
        Prophet: The loaded Prophet model.
    """
    logging.info(f"Loading model from: {filepath}")
    model = joblib.load(filepath)
    logging.info("Model loaded.")
    return model

def make_predictions(model, df, target_column):
    """
    Makes predictions using the trained Prophet model.

    Args:
        model (Prophet): The trained Prophet model.
        df (pd.DataFrame): The DataFrame to make predictions on.
        target_column (str): The name of the target column.

    Returns:
        pd.DataFrame: The DataFrame with predictions.
    """
    logging.info("Making predictions...")
    df = df.reset_index().rename(columns={'Datetime': 'ds'})
    predictions = model.predict(df)
    predictions = predictions[['ds', 'yhat']].rename(columns={'ds': 'Datetime', 'yhat': 'Prediction'})
    predictions = predictions.set_index('Datetime')
    logging.info("Predictions made.")
    return predictions

def evaluate_predictions(predictions, actual, target_column):
    """
    Evaluates the predictions using Mean Absolute Error (MAE) and Root Mean Squared Error (RMSE).

    Args:
        predictions (pd.DataFrame): The DataFrame with predictions.
        actual (pd.DataFrame): The DataFrame with actual values.
        target_column (str): The name of the target column.
    """
    logging.info("Evaluating predictions...")
    mae = mean_absolute_error(actual[target_column], predictions['Prediction'])
    rmse = mean_squared_error(actual[target_column], predictions['Prediction'], squared=False)
    logging.info(f"Mean Absolute Error (MAE): {mae}")
    logging.info(f"Root Mean Squared Error (RMSE): {rmse}")

if __name__ == "__main__":
    transport_data = load_public_transport_data()
    if transport_data is not None:
        target_column = 'Count'  # Replace with the actual target column name
        train_data = transport_data.iloc[:-24]  # Use all but the last 24 hours for training
        test_data = transport_data.iloc[-24:]   # Use the last 24 hours for testing
        preprocessed_train_data = preprocess_data(train_data.copy(), target_column)
        preprocessed_test_data = preprocess_data(test_data.copy(), target_column)
        model = load_model()
        predictions = make_predictions(model, preprocessed_test_data, target_column)
        evaluate_predictions(predictions, test_data, target_column)
    else:
        logging.error("Anomaly detection failed due to data loading error.")
